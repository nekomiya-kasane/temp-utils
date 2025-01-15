#pragma once

#include <_typetraits>
#include <any>
#include <atomic>
#include <concepts>
#include <condition_variable>
#include <coroutine>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <typeindex>
#include <vector>

// Connection properties

class Connection {
 public:
  enum class Type {
    Auto,     // Automatically choose connection type
    Direct,   // Direct call
    Queued,   // Queued connection (async)
    Blocking  // Blocking connection
  };

  enum Priority { Highest = 0, High = 0x1000, Normal = 0x2000, Low = 0x3000, Lowest = 0x4000 };

  struct Properties {
    Type type = Type::Auto;
    Priority priority = Priority::Normal;
    bool enabled = true;
  };

  Connection() = default;
  Connection(std::weak_ptr<SignalBase> signal,
             std::weak_ptr<SlotBase> slot,
             Type type = Type::Auto,
             Priority priority = Priority::Normal);

  void disconnect();
  bool isValid() const;
  void setEnabled(bool enabled);
  bool isEnabled() const;

 private:
  std::weak_ptr<SignalBase> _signal;
  std::weak_ptr<SlotBase> _slot;

  Type _type;
  Priority _priority;
  bool _enabled = true;

  friend class SignalBase;
  friend class SlotBase;
};

// Primary signal template
template<typename Signature, typename ReturnCollector = void> class Signal;

// we assume that adding and removing slots does not often happen
template<typename... Args> class Signal<void(Args...), void> {
 public:
  using SlotFunction = std::function<void(Args...)>;

  template<typename Obj, typename Func>
  Connection connect(Obj *obj, Func &&func, const Connection::Properties &props = {})
  {
    if constexpr (std::is_member_function_pointer_v<std::decay_t<Func>>) {
      return connectMember(obj, std::forward<Func>(func), props);
    }
    else {
      return connectCallable(obj, std::forward<Func>(func), props);
    }
  }

  template<typename Func> Connection connect(Func &&func, const Connection::Properties &props = {})
  {
    return connectCallable(nullptr, std::forward<Func>(func), props);
  }

  void emit(Args... args)
  {
    std::vector<SlotEntry> &activeSlots = _slots;
    // std::vector<SlotEntry> activeSlots;
    //{
    //   std::shared_lock lock(_mutex);
    //   activeSlots = _slots;
    // }

    for (const auto &entry : activeSlots) {
      if (!entry.enabled)
        continue;

      switch (entry.props.type) {
        case Connection::Type::Direct:
          invokeSlot(entry, args...);
          break;
        case Connection::Type::Queued:
          EventLoop::getInstance().postEvent([entry, args...]() { invokeSlot(entry, args...); });
          break;
        case Connection::Type::Blocking: {
          std::promise<void> promise;
          auto future = promise.get_future();
          EventLoop::getInstance().postEvent([entry, args..., &promise]() {
            invokeSlot(entry, args...);
            promise.set_value();
          });
          future.wait();
          break;
        }
        case Connection::Type::Auto:
          if (std::this_thread::get_id() == EventLoop::getInstance().getMainThreadId()) {
            invokeSlot(entry, args...);
          }
          else {
            EventLoop::getInstance().postEvent([entry, args...]() { invokeSlot(entry, args...); });
          }
          break;
      }
    }
  }

  void disconnectAll()
  {
    std::unique_lock lock(_mutex);
    _slots.clear();
  }

 private:
  struct SlotEntry {
    void *obj = nullptr;
    SlotFunction func;
    Connection::Properties props;
    bool enabled = true;
  };

  template<typename Obj, typename Func>
  Connection connectMember(Obj *obj, Func &&func, const Connection::Properties &props)
  {
    auto wrapper = [obj, func](Args... args) {
      if constexpr (HasBeforeSlot<Obj>) {
        obj->beforeSlot();
      }
      (obj->*func)(args...);
      if constexpr (HasAfterSlot<Obj>) {
        obj->afterSlot();
      }
    };
    return addSlot(obj, std::move(wrapper), props);
  }

  template<typename Obj, typename Func>
  Connection connectCallable(Obj *obj, Func &&func, const Connection::Properties &props)
  {
    auto wrapper = [obj, func = std::forward<Func>(func)](Args... args) {
      if constexpr (HasBeforeSlot<Obj>) {
        if (obj)
          obj->beforeSlot();
      }
      func(args...);
      if constexpr (HasAfterSlot<Obj>) {
        if (obj)
          obj->afterSlot();
      }
    };
    return addSlot(obj, std::move(wrapper), props);
  }

  Connection addSlot(void *obj, SlotFunction &&func, const Connection::Properties &props)
  {
    std::unique_lock lock(_mutex);

    auto insertPos = std::find_if(_slots.begin(), _slots.end(), [&props](const SlotEntry &entry) {
      return static_cast<int>(entry.props.priority) > static_cast<int>(props.priority);
    });

    auto it = _slots.insert(insertPos, SlotEntry{obj, std::move(func), props, true});

    return Connection([this, it]() {
      std::unique_lock lock(_mutex);
      if (it < _slots.end()) {
        _slots.erase(it);
      }
    });
  }

  static void invokeSlot(const SlotEntry &entry, Args... args)
  {
    if (entry.enabled && entry.func) {
      entry.func(args...);
    }
  }

  std::vector<SlotEntry> _slots // todo: use list
  mutable std::shared_mutex _mutex;
};
