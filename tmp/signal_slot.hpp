#pragma once

#include <concepts>
#include <functional>
#include <map>
#include <vector>
#include <memory>
#include <any>
#include <typeindex>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <coroutine>
#include <type_traits>
#include <optional>
#include <variant>

namespace signal_slot {

// Connection properties
struct ConnectionProperties {
    enum class Type {
        Auto,       // Automatically choose connection type
        Direct,     // Direct call
        Queued,     // Queued connection (async)
        Blocking    // Blocking connection
    };

    enum class Priority {
        Highest = 0,
        High = 1,
        Normal = 2,
        Low = 3,
        Lowest = 4
    };

    Type type = Type::Auto;
    Priority priority = Priority::Normal;
    bool enabled = true;
};

// Return value collection strategies
struct ReturnStrategy {
    enum class Type {
        All,            // Collect all return values
        First,          // Only keep first non-empty return value
        Last,           // Only keep last non-empty return value
        FirstValid,     // Keep first value that satisfies a predicate
        Reduce         // Reduce all values using a binary operation
    };
};

// Connection class
class Connection {
public:
    Connection() = default;
    Connection(const Connection&) = delete;
    Connection(Connection&&) noexcept = default;
    Connection& operator=(const Connection&) = delete;
    Connection& operator=(Connection&&) noexcept = default;

    void disconnect();
    bool isValid() const { return !disconnected_; }
    void setEnabled(bool enabled) { enabled_ = enabled; }
    bool isEnabled() const { return enabled_; }

private:
    template<typename... Args>
    friend class Signal;

    Connection(std::function<void()> disconnector)
        : disconnector_(std::move(disconnector)) {}

    std::function<void()> disconnector_;
    bool disconnected_ = false;
    bool enabled_ = true;
};

// Concepts for member function detection
template<typename T>
concept HasBeforeSlot = requires(T& t) {
    { t.beforeSlot() } -> std::same_as<void>;
};

template<typename T>
concept HasAfterSlot = requires(T& t) {
    { t.afterSlot() } -> std::same_as<void>;
};

// Helper type traits
template<typename T>
struct function_traits : function_traits<decltype(&T::operator())> {};

template<typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    using return_type = R;
    using argument_types = std::tuple<Args...>;
};

template<typename C, typename R, typename... Args>
struct function_traits<R(C::*)(Args...)> {
    using return_type = R;
    using argument_types = std::tuple<Args...>;
};

template<typename C, typename R, typename... Args>
struct function_traits<R(C::*)(Args...) const> {
    using return_type = R;
    using argument_types = std::tuple<Args...>;
};

// Primary signal template
template<typename Signature, typename ReturnCollector = void>
class Signal;

// Specialization for void return type
template<typename... Args>
class Signal<void(Args...), void> {
public:
    using SlotFunction = std::function<void(Args...)>;

    template<typename Obj, typename Func>
    Connection connect(Obj* obj, Func&& func, 
                      const ConnectionProperties& props = ConnectionProperties{}) {
        if constexpr (std::is_member_function_pointer_v<std::decay_t<Func>>) {
            return connectMember(obj, std::forward<Func>(func), props);
        } else {
            return connectCallable(obj, std::forward<Func>(func), props);
        }
    }

    template<typename Func>
    Connection connect(Func&& func, 
                      const ConnectionProperties& props = ConnectionProperties{}) {
        return connectCallable(nullptr, std::forward<Func>(func), props);
    }

    void emit(Args... args) {
        std::vector<SlotEntry> activeSlots;
        {
            std::shared_lock lock(mutex_);
            activeSlots = slots_;
        }

        for (const auto& entry : activeSlots) {
            if (!entry.enabled) continue;

            switch (entry.props.type) {
                case ConnectionProperties::Type::Direct:
                    invokeSlot(entry, args...);
                    break;
                case ConnectionProperties::Type::Queued:
                    EventLoop::getInstance().postEvent([entry, args...]() {
                        invokeSlot(entry, args...);
                    });
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
                    } else {
                        EventLoop::getInstance().postEvent([entry, args...]() {
                            invokeSlot(entry, args...);
                        });
                    }
                    break;
            }
        }
    }

    void disconnectAll() {
        std::unique_lock lock(mutex_);
        slots_.clear();
    }

private:
    struct SlotEntry {
        void* obj = nullptr;
        SlotFunction func;
        ConnectionProperties props;
        bool enabled = true;
    };

    template<typename Obj, typename Func>
    Connection connectMember(Obj* obj, Func&& func, const ConnectionProperties& props) {
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
    Connection connectCallable(Obj* obj, Func&& func, const ConnectionProperties& props) {
        auto wrapper = [obj, func = std::forward<Func>(func)](Args... args) {
            if constexpr (HasBeforeSlot<Obj>) {
                if (obj) obj->beforeSlot();
            }
            func(args...);
            if constexpr (HasAfterSlot<Obj>) {
                if (obj) obj->afterSlot();
            }
        };
        return addSlot(obj, std::move(wrapper), props);
    }

    Connection addSlot(void* obj, SlotFunction&& func, const ConnectionProperties& props) {
        std::unique_lock lock(mutex_);
        
        auto insertPos = std::find_if(slots_.begin(), slots_.end(),
            [&props](const SlotEntry& entry) {
                return static_cast<int>(entry.props.priority) > 
                       static_cast<int>(props.priority);
            });

        auto it = slots_.insert(insertPos, 
            SlotEntry{obj, std::move(func), props, true});

        return Connection([this, it]() {
            std::unique_lock lock(mutex_);
            if (it < slots_.end()) {
                slots_.erase(it);
            }
        });
    }

    static void invokeSlot(const SlotEntry& entry, Args... args) {
        if (entry.enabled && entry.func) {
            entry.func(args...);
        }
    }

    std::vector<SlotEntry> slots_;
    mutable std::shared_mutex mutex_;
};

// Specialization for non-void return type
template<typename R, typename... Args, typename ReturnCollector>
class Signal<R(Args...), ReturnCollector> {
public:
    using SlotFunction = std::function<R(Args...)>;
    using ReturnType = R;
    using ReturnCollection = std::vector<R>;

    template<typename Obj, typename Func>
    Connection connect(Obj* obj, Func&& func, 
                      const ConnectionProperties& props = ConnectionProperties{}) {
        if constexpr (std::is_member_function_pointer_v<std::decay_t<Func>>) {
            return connectMember(obj, std::forward<Func>(func), props);
        } else {
            return connectCallable(obj, std::forward<Func>(func), props);
        }
    }

    template<typename Func>
    Connection connect(Func&& func, 
                      const ConnectionProperties& props = ConnectionProperties{}) {
        return connectCallable(nullptr, std::forward<Func>(func), props);
    }

    // Emit with return value collection
    ReturnCollection emit(Args... args) {
        std::vector<SlotEntry> activeSlots;
        {
            std::shared_lock lock(mutex_);
            activeSlots = slots_;
        }

        ReturnCollection results;
        results.reserve(activeSlots.size());

        for (const auto& entry : activeSlots) {
            if (!entry.enabled) continue;

            if constexpr (std::is_same_v<ReturnCollector, 
                         ReturnStrategy::Type::First>) {
                auto result = invokeSlot(entry, args...);
                if (result.has_value()) {
                    results.push_back(std::move(*result));
                    break;
                }
            } 
            else if constexpr (std::is_same_v<ReturnCollector, 
                              ReturnStrategy::Type::Last>) {
                auto result = invokeSlot(entry, args...);
                if (result.has_value()) {
                    results = {std::move(*result)};
                }
            }
            else {
                // Default: collect all results
                if (auto result = invokeSlot(entry, args...)) {
                    results.push_back(std::move(*result));
                }
            }
        }

        return results;
    }

    // Emit with custom collector
    template<typename Collector>
    auto emit_with(Args... args, Collector&& collector) {
        std::vector<SlotEntry> activeSlots;
        {
            std::shared_lock lock(mutex_);
            activeSlots = slots_;
        }

        return collector(activeSlots, args...);
    }

    void disconnectAll() {
        std::unique_lock lock(mutex_);
        slots_.clear();
    }

private:
    struct SlotEntry {
        void* obj = nullptr;
        SlotFunction func;
        ConnectionProperties props;
        bool enabled = true;
    };

    template<typename Obj, typename Func>
    Connection connectMember(Obj* obj, Func&& func, 
                           const ConnectionProperties& props) {
        auto wrapper = [obj, func](Args... args) -> R {
            if constexpr (HasBeforeSlot<Obj>) {
                obj->beforeSlot();
            }
            R result = (obj->*func)(args...);
            if constexpr (HasAfterSlot<Obj>) {
                obj->afterSlot();
            }
            return result;
        };
        return addSlot(obj, std::move(wrapper), props);
    }

    template<typename Obj, typename Func>
    Connection connectCallable(Obj* obj, Func&& func, 
                             const ConnectionProperties& props) {
        auto wrapper = [obj, func = std::forward<Func>(func)](Args... args) -> R {
            if constexpr (HasBeforeSlot<Obj>) {
                if (obj) obj->beforeSlot();
            }
            R result = func(args...);
            if constexpr (HasAfterSlot<Obj>) {
                if (obj) obj->afterSlot();
            }
            return result;
        };
        return addSlot(obj, std::move(wrapper), props);
    }

    Connection addSlot(void* obj, SlotFunction&& func, 
                      const ConnectionProperties& props) {
        std::unique_lock lock(mutex_);
        
        auto insertPos = std::find_if(slots_.begin(), slots_.end(),
            [&props](const SlotEntry& entry) {
                return static_cast<int>(entry.props.priority) > 
                       static_cast<int>(props.priority);
            });

        auto it = slots_.insert(insertPos, 
            SlotEntry{obj, std::move(func), props, true});

        return Connection([this, it]() {
            std::unique_lock lock(mutex_);
            if (it < slots_.end()) {
                slots_.erase(it);
            }
        });
    }

    std::optional<R> invokeSlot(const SlotEntry& entry, Args... args) {
        if (entry.enabled && entry.func) {
            try {
                return entry.func(args...);
            } catch (...) {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }

    std::vector<SlotEntry> slots_;
    mutable std::shared_mutex mutex_;
};

// Helper type aliases for common return collection strategies
template<typename Signature>
using SignalAll = Signal<Signature, ReturnStrategy::Type::All>;

template<typename Signature>
using SignalFirst = Signal<Signature, ReturnStrategy::Type::First>;

template<typename Signature>
using SignalLast = Signal<Signature, ReturnStrategy::Type::Last>;

// Custom collectors
template<typename R>
struct FirstValidCollector {
    template<typename F>
    FirstValidCollector(F&& validator) : isValid(std::forward<F>(validator)) {}

    template<typename SlotEntry, typename... Args>
    std::optional<R> operator()(const std::vector<SlotEntry>& slots, Args&&... args) {
        for (const auto& entry : slots) {
            if (auto result = entry.func(std::forward<Args>(args)...)) {
                if (isValid(*result)) {
                    return result;
                }
            }
        }
        return std::nullopt;
    }

    std::function<bool(const R&)> isValid;
};

template<typename R>
struct ReduceCollector {
    template<typename F>
    ReduceCollector(R initial, F&& reducer) 
        : initial(std::move(initial)), reduce(std::forward<F>(reducer)) {}

    template<typename SlotEntry, typename... Args>
    R operator()(const std::vector<SlotEntry>& slots, Args&&... args) {
        R result = initial;
        for (const auto& entry : slots) {
            if (auto value = entry.func(std::forward<Args>(args)...)) {
                result = reduce(std::move(result), std::move(*value));
            }
        }
        return result;
    }

    R initial;
    std::function<R(R&&, R&&)> reduce;
};

// 事件循环类
class EventLoop {
public:
    static EventLoop& getInstance() {
        static EventLoop instance;
        return instance;
    }
    
    void start();
    void stop();
    void postEvent(std::function<void()> event);
    bool isRunning() const { return running_; }
    std::thread::id getMainThreadId() const { return mainThreadId_; }
    
private:
    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;
    
    void processEvents();
    
    std::queue<std::function<void()>> eventQueue_;
    std::mutex queueMutex_;
    std::condition_variable condition_;
    std::thread eventThread_;
    std::atomic<bool> running_{false};
    std::thread::id mainThreadId_;
};

} // namespace signal_slot
