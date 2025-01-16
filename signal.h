#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "basic_concepts.h"

template<typename R> struct Task {
  std::function<R()> func;
  int priority;
  uint64_t sequence;

  bool operator<(const Task &other) const
  {
    if (priority != other.priority)
      return priority < other.priority;
    return sequence > other.sequence;
  }
};

using ConnectionId = uint64_t;

enum class ThreadSafety {
  None,      // 无线程安全保护
  Shared,    // 使用共享锁
  Exclusive  // 使用互斥锁
};

enum class ConnectionType {
  Direct,          // 直接调用
  Queued,          // 放入队列
  BlockingQueued,  // 阻塞队列
  Auto,
};

struct ConnectionOptions {
  ConnectionType type = ConnectionType::Direct;
  int priority = 0;
  bool autoDisconnect = false;
};

class SlotBase {
 public:
  virtual ~SlotBase() = default;
  virtual bool isValid() const = 0;
  virtual void disconnect() = 0;
};

template<typename Slot, typename... Args> class SlotWrapper : public SlotBase {
  std::weak_ptr<void> weakPtr;
  Slot slot;

 public:
  template<typename T>
  SlotWrapper(T *obj, Slot &&s) : weakPtr(obj->shared_from_this()), slot(std::forward<Slot>(s))
  {
  }

  SlotWrapper(Slot &&s) : slot(std::forward<Slot>(s)) {}

  bool isValid() const override
  {
    return !weakPtr.expired();
  }

  void disconnect() override
  {
    weakPtr.reset();
  }

  template<typename... CallArgs> auto invoke(CallArgs &&...args)
  {
    if (weakPtr.expired())
      return;
    return slot(std::forward<CallArgs>(args)...);
  }
};

class SignalBase {
 protected:
  static ConnectionId generateConnectionId()
  {
    static std::atomic<ConnectionId> nextId{1};
    return nextId++;
  }
};

template<typename Signature, ThreadSafety Safety = ThreadSafety::None> class Signal;

template<typename R, typename... Args, ThreadSafety Safety>
class Signal<R(Args...), Safety> : public SignalBase {
 public:
  using ResultType = R;
  using SlotType = std::function<R(Args...)>;

 private:
  struct Connection {
    std::shared_ptr<SlotBase> slotWrapper;
    SlotType slot;
    ConnectionType type;
    int priority;
    bool enabled;
    bool autoDisconnect;

    Connection(SlotType s, std::shared_ptr<SlotBase> wrapper, const ConnectionOptions &opts)
        : slot(std::move(s)),
          slotWrapper(std::move(wrapper)),
          type(opts.type),
          priority(opts.priority),
          enabled(true),
          autoDisconnect(opts.autoDisconnect)
    {
    }
  };

  std::unordered_map<ConnectionId, Connection> connections;
  std::priority_queue<Task<R>> taskQueue;

  using MutexType =
      std::conditional_t<Safety == ThreadSafety::Shared, std::shared_mutex, std::mutex>;
  mutable MutexType mutex;

  template<typename Func> auto withLock(Func &&f) const
  {
    if constexpr (Safety == ThreadSafety::None) {
      return f();
    }
    else if constexpr (Safety == ThreadSafety::Shared) {
      std::shared_lock lock(mutex);
      return f();
    }
    else {
      std::lock_guard lock(mutex);
      return f();
    }
  }

  template<typename Func> auto withExclusiveLock(Func &&f)
  {
    if constexpr (Safety == ThreadSafety::None) {
      return f();
    }
    else {
      std::lock_guard lock(mutex);
      return f();
    }
  }

 public:
  template<typename C, typename Func>
  ConnectionId connect(C *obj, Func &&func, const ConnectionOptions &opts = {})
  {
    static_assert(std::is_base_of_v<std::enable_shared_from_this<C>, C>,
                  "Class must inherit from std::enable_shared_from_this");

    auto wrapper = std::make_shared<SlotWrapper<Func, Args...>>(obj, std::forward<Func>(func));

    return withExclusiveLock([&] {
      auto id = generateConnectionId();
      auto slot = [weak = std::weak_ptr<SlotWrapper<Func, Args...>>(wrapper), func, obj](
                      Args... args) -> R {
        if (auto strong = weak.lock()) {
          return (obj->*func)(std::forward<Args>(args)...);
        }
        return R();
      };
      connections.emplace(id, Connection(std::move(slot), wrapper, opts));
      return id;
    });
  }

  template<typename Func> ConnectionId connect(Func &&func, const ConnectionOptions &opts = {})
  {
    auto wrapper = std::make_shared<SlotWrapper<Func, Args...>>(std::forward<Func>(func));

    return withExclusiveLock([&] {
      auto id = generateConnectionId();
      connections.emplace(id, Connection(std::forward<Func>(func), wrapper, opts));
      return id;
    });
  }

  template<typename Container>
  std::vector<ConnectionId> connectAll(const Container &slots, const ConnectionOptions &opts = {})
  {
    std::vector<ConnectionId> ids;
    ids.reserve(std::size(slots));

    for (const auto &slot : slots) {
      ids.push_back(connect(slot, opts));
    }
    return ids;
  }

  void disconnectAll(const std::vector<ConnectionId> &ids)
  {
    for (auto id : ids) {
      disconnect(id);
    }
  }

  void pauseAll()
  {
    withExclusiveLock([&] {
      for (auto &[id, conn] : connections) {
        conn.enabled = false;
      }
    });
  }

  void resumeAll()
  {
    withExclusiveLock([&] {
      for (auto &[id, conn] : connections) {
        conn.enabled = true;
      }
    });
  }

  void flush()
  {
    while (hasPendingTasks()) {
      processQueue();
    }
  }

  void cleanup()
  {
    withExclusiveLock([&] {
      auto it = connections.begin();
      while (it != connections.end()) {
        if (it->second.autoDisconnect &&
            (!it->second.slotWrapper || !it->second.slotWrapper->isValid()))
        {
          it = connections.erase(it);
        }
        else {
          ++it;
        }
      }
    });
  }

  template<typename... EmitArgs>
    requires std::same_as<R, void>
  void emit(EmitArgs &&...args) const
  {
    withLock([&] {
      for (const auto &[id, conn] : connections) {
        if (!conn.enabled)
          continue;

        auto task = [slot = conn.slot, ... args = std::forward<EmitArgs>(args)]() {
          return slot(std::forward<EmitArgs>(args)...);
        };

        if (conn.type == ConnectionType::Direct) {
          task();
        }
        else {
          taskQueue.push(Task<R>{std::move(task), conn.priority, id});

          if (conn.type == ConnectionType::BlockingQueued) {
            processQueue();
          }
        }
      }
    });
  }

  template<typename... EmitArgs> std::vector<R> emit(EmitArgs &&...args) const
  {
    std::vector<R> results;

    withLock([&] {
      for (const auto &[id, conn] : connections) {
        if (!conn.enabled)
          continue;

        auto task = [slot = conn.slot, ... args = std::forward<EmitArgs>(args)]() {
          return slot(std::forward<EmitArgs>(args)...);
        };

        if (conn.type == ConnectionType::Direct) {
          if constexpr (!std::is_void_v<R>) {
            results.push_back(task());
          }
          else {
            task();
          }
        }
        else {
          taskQueue.push(Task<R>{std::move(task), conn.priority, id});

          if (conn.type == ConnectionType::BlockingQueued) {
            processQueue();
          }
        }
      }
    });

    return results;
  }

  //template<typename... EmitArgs> std::optional<R> emitTo(ConnectionId id, EmitArgs &&...args) const
  //{
  //  return withLock([&]() -> std::optional<R> {
  //    auto it = connections.find(id);
  //    if (it == connections.end() || !it->second.enabled) {
  //      return std::nullopt;
  //    }

  //    if (it->second.type == ConnectionType::Direct) {
  //      return it->second.slot(std::forward<EmitArgs>(args)...);
  //    }

  //    auto task = [slot = it->second.slot, ... args = std::forward<EmitArgs>(args)]() {
  //      return slot(std::forward<EmitArgs>(args)...);
  //    };
  //    taskQueue.push(Task<R>{std::move(task), it->second.priority, id});

  //    if (it->second.type == ConnectionType::BlockingQueued) {
  //      processQueue();
  //      return R();  // 队列处理完成后的默认值
  //    }
  //    return std::nullopt;
  //  });
  //}

  void processQueue()
  {
    while (!taskQueue.empty()) {
      auto task = std::move(taskQueue.top());
      taskQueue.pop();

      if constexpr (!std::is_void_v<R>) {
        task.func();
      }
      else {
        task.func();
      }
    }
  }

  bool isConnected(ConnectionId id) const
  {
    return withLock([&] { return connections.contains(id); });
  }

  size_t connectionCount() const
  {
    return withLock([&] { return connections.size(); });
  }

  bool hasPendingTasks() const
  {
    return withLock([&] { return !taskQueue.empty(); });
  }

  bool disconnect(ConnectionId id)
  {
    return withExclusiveLock([&] { return connections.erase(id) > 0; });
  }

  void disconnectAll()
  {
    withExclusiveLock([&] {
      connections.clear();
      std::priority_queue<Task<R>> empty;
      taskQueue.swap(empty);
    });
  }

  bool setEnabled(ConnectionId id, bool enabled)
  {
    return withExclusiveLock([&] {
      auto it = connections.find(id);
      if (it != connections.end()) {
        it->second.enabled = enabled;
        return true;
      }
      return false;
    });
  }
};

template<typename Signature, ThreadSafety Safety = ThreadSafety::None> auto makeSignal()
{
  return Signal<Signature, Safety>();
}

class ScopedConnection {
  Signal<void()> *signal;
  ConnectionId id;

 public:
  ScopedConnection(Signal<void()> *s, ConnectionId c) : signal(s), id(c) {}

  ~ScopedConnection()
  {
    if (signal)
      signal->disconnect(id);
  }

  ScopedConnection(const ScopedConnection &) = delete;
  ScopedConnection &operator=(const ScopedConnection &) = delete;

  ScopedConnection(ScopedConnection &&other) noexcept : signal(other.signal), id(other.id)
  {
    other.signal = nullptr;
  }

  ScopedConnection &operator=(ScopedConnection &&other) noexcept
  {
    if (this != &other) {
      if (signal)
        signal->disconnect(id);
      signal = other.signal;
      id = other.id;
      other.signal = nullptr;
    }
    return *this;
  }
};
