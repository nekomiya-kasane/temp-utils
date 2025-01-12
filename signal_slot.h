#pragma once

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
#include <type_traits>
#include <typeindex>
#include <vector>

// Connection properties
struct ConnectionProperties {
  enum class Type {
    Auto,     // Automatically choose connection type
    Direct,   // Direct call
    Queued,   // Queued connection (async)
    Blocking  // Blocking connection
  };

  enum Priority { Highest = 0, High = 0x1000, Normal = 0x2000, Low = 0x3000, Lowest = 0x4000 };

  Type type = Type::Auto;
  Priority priority = Priority::Normal;
  bool enabled = true;
};

// Primary signal template
template<typename Signature, typename ReturnCollector = void> class Signal;

// we assume that adding and removing slots does not often happen
template<typename... Args> class Signal<void(Args...), void> {
 public:
  using SlotFunction = std::function<void(Args...)>;

  template<typename Obj, typename Func>
  Connection connect(Obj *obj,
                     Func &&func,
                     const ConnectionProperties &props = ConnectionProperties{})
  {
    if constexpr (std::is_member_function_pointer_v<std::decay_t<Func>>) {
      return connectMember(obj, std::forward<Func>(func), props);
    }
    else {
      return connectCallable(obj, std::forward<Func>(func), props);
    }
  }

  template<typename Func>
  Connection connect(Func &&func, const ConnectionProperties &props = ConnectionProperties{})
  {
    return connectCallable(nullptr, std::forward<Func>(func), props);
  }

  void emit(Args... args)
  {
    std::vector<SlotEntry> activeSlots;
    {
      std::shared_lock lock(mutex_);
      activeSlots = slots_;
    }

    for (const auto &entry : activeSlots) {
      if (!entry.enabled)
        continue;

      switch (entry.props.type) {
        case ConnectionProperties::Type::Direct:
          invokeSlot(entry, args...);
          break;
        case ConnectionProperties::Type::Queued:
          EventLoop::getInstance().postEvent([entry, args...]() { invokeSlot(entry, args...); });
          break;
        case ConnectionProperties::Type::Blocking: {
          std::promise<void> promise;
          auto future = promise.get_future();
          EventLoop::getInstance().postEvent([entry, args..., &promise]() {
            invokeSlot(entry, args...);
            promise.set_value();
          });
          future.wait();
          break;
        }
        case ConnectionProperties::Type::Auto:
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
    std::unique_lock lock(mutex_);
    slots_.clear();
  }

 private:
  struct SlotEntry {
    void *obj = nullptr;
    SlotFunction func;
    ConnectionProperties props;
    bool enabled = true;
  };

  template<typename Obj, typename Func>
  Connection connectMember(Obj *obj, Func &&func, const ConnectionProperties &props)
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
  Connection connectCallable(Obj *obj, Func &&func, const ConnectionProperties &props)
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

  Connection addSlot(void *obj, SlotFunction &&func, const ConnectionProperties &props)
  {
    std::unique_lock lock(mutex_);

    auto insertPos = std::find_if(slots_.begin(), slots_.end(), [&props](const SlotEntry &entry) {
      return static_cast<int>(entry.props.priority) > static_cast<int>(props.priority);
    });

    auto it = slots_.insert(insertPos, SlotEntry{obj, std::move(func), props, true});

    return Connection([this, it]() {
      std::unique_lock lock(mutex_);
      if (it < slots_.end()) {
        slots_.erase(it);
      }
    });
  }

  static void invokeSlot(const SlotEntry &entry, Args... args)
  {
    if (entry.enabled && entry.func) {
      entry.func(args...);
    }
  }

  std::vector<SlotEntry> slots_;
  mutable std::shared_mutex mutex_;
};
