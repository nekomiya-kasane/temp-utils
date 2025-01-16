#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "basic_concepts.h"

#include <iostream>
#include <ostream>

struct ConnectionConfig {
  std::string name;
  bool disabled = false;
  int priority = 0;
};

// Return value collection strategies
struct ReturnStrategy {
  enum class Type {
    All,         // Collect all return values
    First,       // Only keep first non-empty return value
    Last,        // Only keep last non-empty return value
    FirstValid,  // Keep first value that satisfies a predicate
    Reduce       // Reduce all values using a binary operation
  };
};

namespace traits {

  template<typename T>
  concept HasBeforeSlot = requires(T &t) {
    {
      t.beforeSlot()
    } -> std::same_as<void>;
  };

  template<typename T>
  concept HasAfterSlot = requires(T &t) {
    {
      t.afterSlot()
    } -> std::same_as<void>;
  };

}  // namespace traits

template<typename... Args> class Signal<void(Args...)> {
 public:
  using function_type = std::function<void(Args...)>;

  enum class InvokeType {
    Auto,     // Automatically choose connection type
    Direct,   // Direct call
    Queued,   // Queued connection (async)
    Blocking  // Blocking connection
  };

  struct SlotEntry {
    std::string name;

    InvokeType type = InvokeType::Auto;

    void *obj = nullptr;
    function_type func;

    int priority = 0;
    bool enabled = true;

    auto operator<=>(const SlotEntry &other) const
    {
      if (name == other.name) {
        return std::strong_ordering::equal;
      }
      return priority <=> other.priority;
    }
  };

  template<typename Obj, typename Func>
    requires std::is_pointer_v<std::remove_cv_t<Obj>> || std::is_null_pointer_v<Obj>
  bool connect(Obj obj, Func &&func, const ConnectionConfig &props = ConnectionConfig{})
  {
    if constexpr (std::is_member_function_pointer_v<std::decay_t<Func>>) {
      return connectMember(obj, std::forward<Func>(func), props);
    }
    else {
      return connectCallable(obj, std::forward<Func>(func), props);
    }
  }

  template<typename Obj, typename Func>
    requires std::is_pointer_v<std::remove_cv_t<Obj>> || std::is_null_pointer_v<Obj>
  bool connectMember(Obj obj, Func &&func, const ConnectionConfig &props, const std::string &name)
  {
    auto wrapper = [obj, func](Args... args) {
      if constexpr (traits::HasBeforeSlot<Obj>) {
        obj->beforeSlot();
      }
      (obj->*func)(args...);
      if constexpr (traits::HasAfterSlot<Obj>) {
        obj->afterSlot();
      }
    };
    return addSlot(obj, std::move(wrapper), props, name);
  }

  template<typename Obj, typename Func>
    requires std::is_pointer_v<std::remove_cv_t<Obj>> || std::is_null_pointer_v<Obj>
  bool connectCallable(Obj obj, Func &&func, const ConnectionConfig &props)
  {
    auto wrapper = [obj, func = std::forward<Func>(func)](Args... args) {
      if constexpr (traits::HasBeforeSlot<Obj>) {
        if (obj)
          obj->beforeSlot();
      }
      func(args...);
      if constexpr (traits::HasAfterSlot<Obj>) {
        if (obj)
          obj->afterSlot();
      }
    };
    return addSlot(obj, std::move(wrapper), props);
  }

  bool addSlot(void *obj, function_type &&func, const ConnectionConfig &props)
  {
    std::unique_lock lock(mutex_);

    // Check for duplicates
#ifdef _DEBUG
    auto it = std::find_if(slots_.begin(), slots_.end(), [&](const SlotEntry &entry) {
      return entry.name == props.name ||
             (entry.obj == obj && entry.func.target_type() == func.target_type());
    });

    if (it != slots_.end()) {
      std::cout << "Duplicate slot detected. Not adding." << std::endl;
      return false;
    }
#endif

    slots_.push({obj, std::move(func), std::move(props), true});
    return true;
  }

  bool disconnect(const std::string &name = "")
  {
    std::unique_lock lock(mutex_);
    bool found = false;
    std::priority_queue<SlotEntry> newQueue;

    while (!slots_.empty()) {
      auto entry = slots_.top();
      slots_.pop();

      if (entry.obj != obj || (name != "" && entry.name != name)) {
        newQueue.push(std::move(entry));
      }
      else {
        found = true;
      }
    }

    slots_ = std::move(newQueue);
    return found;
  }

  template<typename... Args> void emit(Args... args)
  {
    std::shared_lock lock(mutex_);
    auto tempQueue = slots_;
    while (!tempQueue.empty()) {
      const auto &entry = tempQueue.top();
      if (entry.enabled) {
        entry.func(args...);
      }
      tempQueue.pop();
    }
  }

  template<typename... Args> void operator()(Args... args)
  {
    emit(std::forward<Args>(args)...);
  }

 private:
  std::priority_queue<SlotEntry> slots_;
  mutable std::shared_mutex mutex_;
};
