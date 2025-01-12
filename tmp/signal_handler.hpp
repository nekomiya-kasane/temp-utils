/**
 * @file signal_handler.hpp
 * @brief Signal handler implementation with flexible callback support
 * @~chinese
 * @brief 具有灵活回调支持的信号处理器实现
 */

#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <atomic>
#include <csignal>
#include <memory>
#include <source_location>
#include <string_view>
#include <concepts>

namespace sig {

/**
 * @brief Signal handler callback type
 * @~chinese
 * @brief 信号处理器回调类型
 */
using SignalCallback = std::function<void(int)>;

/**
 * @brief Concept for signal handlers
 * @~chinese
 * @brief 信号处理器的概念
 */
template<typename T>
concept SignalCallable = requires(T t, int signal) {
    { t(signal) } -> std::same_as<void>;
};

/**
 * @brief Signal information structure
 * @~chinese
 * @brief 信号信息结构
 */
struct SignalInfo {
    int signal_number;           ///< Signal number
    std::string_view name;       ///< Signal name
    std::string_view description;///< Signal description
    bool is_default_fatal;       ///< Whether signal is fatal by default
};

/**
 * @brief Signal handler class
 * @~chinese
 * @brief 信号处理器类
 */
class SignalHandler {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to singleton instance
     * @~chinese
     * @brief 获取单例实例
     * @return 单例实例的引用
     */
    static SignalHandler& instance();

    /**
     * @brief Register a callback for a specific signal
     * @param signal_number Signal number to handle
     * @param callback Callback function
     * @param loc Source location for debugging
     * @return true if registration successful
     * @~chinese
     * @brief 为特定信号注册回调
     * @param signal_number 要处理的信号号码
     * @param callback 回调函数
     * @param loc 用于调试的源代码位置
     * @return 如果注册成功则返回true
     */
    template<SignalCallable F>
    bool register_handler(int signal_number, F&& callback,
                         const std::source_location& loc = std::source_location::current()) {
        return register_handler_impl(signal_number, std::forward<F>(callback), loc);
    }

    /**
     * @brief Register a callback for multiple signals
     * @param signals List of signal numbers
     * @param callback Callback function
     * @param loc Source location for debugging
     * @return true if all registrations successful
     * @~chinese
     * @brief 为多个信号注册回调
     * @param signals 信号号码列表
     * @param callback 回调函数
     * @param loc 用于调试的源代码位置
     * @return 如果所有注册都成功则返回true
     */
    template<SignalCallable F>
    bool register_handler(const std::vector<int>& signals, F&& callback,
                         const std::source_location& loc = std::source_location::current()) {
        bool success = true;
        for (int sig : signals) {
            success &= register_handler_impl(sig, callback, loc);
        }
        return success;
    }

    /**
     * @brief Unregister a signal handler
     * @param signal_number Signal number to unregister
     * @return true if unregistration successful
     * @~chinese
     * @brief 取消注册信号处理器
     * @param signal_number 要取消注册的信号号码
     * @return 如果取消注册成功则返回true
     */
    bool unregister_handler(int signal_number);

    /**
     * @brief Signal blocker class for RAII-style signal blocking
     * @~chinese
     * @brief RAII风格的信号阻塞器类
     */
    class SignalBlocker {
    public:
        /**
         * @brief Construct a new Signal Blocker
         * @param signals Signals to block
         * @~chinese
         * @brief 构造新的信号阻塞器
         * @param signals 要阻塞的信号
         */
        explicit SignalBlocker(const std::vector<int>& signals);
        
        /**
         * @brief Destroy the Signal Blocker
         * @~chinese
         * @brief 销毁信号阻塞器
         */
        ~SignalBlocker();
    private:
        sigset_t old_mask_;
    };

    /**
     * @brief Check if a signal is currently being handled
     * @param signal_number Signal number to check
     * @return true if signal is being handled
     * @~chinese
     * @brief 检查信号是否正在被处理
     * @param signal_number 要检查的信号号码
     * @return 如果信号正在被处理则返回true
     */
    bool is_handling_signal(int signal_number) const;

    /**
     * @brief Get information about a signal
     * @param signal_number Signal number
     * @return Signal information
     * @~chinese
     * @brief 获取信号的信息
     * @param signal_number 信号号码
     * @return 信号信息
     */
    static SignalInfo get_signal_info(int signal_number);

    /**
     * @brief Reset handler to default for a signal
     * @param signal_number Signal number
     * @return true if reset successful
     * @~chinese
     * @brief 将信号处理器重置为默认
     * @param signal_number 信号号码
     * @return 如果重置成功则返回true
     */
    bool reset_to_default(int signal_number);

    /**
     * @brief Reset all handlers to default
     * @~chinese
     * @brief 将所有处理器重置为默认
     */
    void reset_all();

    /**
     * @brief Check if a signal is blocked
     * @param signal_number Signal number
     * @return true if signal is blocked
     * @~chinese
     * @brief 检查信号是否被阻塞
     * @param signal_number 信号号码
     * @return 如果信号被阻塞则返回true
     */
    bool is_signal_blocked(int signal_number) const;

    /**
     * @brief Get all registered signals
     * @return Vector of registered signal numbers
     * @~chinese
     * @brief 获取所有已注册的信号
     * @return 已注册信号号码的向量
     */
    std::vector<int> get_registered_signals() const;

    /**
     * @brief Get the source location where a signal handler was registered
     * @param signal_number Signal number
     * @return Source location
     * @~chinese
     * @brief 获取信号处理器注册时的源代码位置
     * @param signal_number 信号号码
     * @return 源代码位置
     */
    std::source_location get_handler_location(int signal_number) const;

    // Disable copy and move
    SignalHandler(const SignalHandler&) = delete;
    SignalHandler& operator=(const SignalHandler&) = delete;
    SignalHandler(SignalHandler&&) = delete;
    SignalHandler& operator=(SignalHandler&&) = delete;

private:
    SignalHandler();
    ~SignalHandler();

    bool register_handler_impl(int signal_number, SignalCallback callback,
                             const std::source_location& loc);
    static void signal_handler(int signal);
    void handle_signal(int signal);

    mutable std::mutex mutex_;
    std::unordered_map<int, SignalCallback> handlers_;
    std::unordered_map<int, std::source_location> handler_locations_;
    std::unordered_map<int, std::atomic<bool>> handling_signals_;
    std::unordered_map<int, struct sigaction> original_handlers_;

    static SignalHandler* instance_;
    static std::mutex instance_mutex_;
};

/**
 * @brief RAII-style signal handler registration
 * @~chinese
 * @brief RAII风格的信号处理器注册
 */
template<SignalCallable F>
class ScopedSignalHandler {
public:
    /**
     * @brief Construct a new Scoped Signal Handler
     * @param signal_number Signal number
     * @param callback Callback function
     * @~chinese
     * @brief 构造新的作用域信号处理器
     * @param signal_number 信号号码
     * @param callback 回调函数
     */
    ScopedSignalHandler(int signal_number, F&& callback)
        : signal_number_(signal_number) {
        SignalHandler::instance().register_handler(signal_number, std::forward<F>(callback));
    }

    /**
     * @brief Destroy the Scoped Signal Handler
     * @~chinese
     * @brief 销毁作用域信号处理器
     */
    ~ScopedSignalHandler() {
        SignalHandler::instance().reset_to_default(signal_number_);
    }

private:
    int signal_number_;
};

/**
 * @brief Helper function to create a scoped signal handler
 * @param signal_number Signal number
 * @param callback Callback function
 * @return Scoped signal handler
 * @~chinese
 * @brief 创建作用域信号处理器的辅助函数
 * @param signal_number 信号号码
 * @param callback 回调函数
 * @return 作用域信号处理器
 */
template<SignalCallable F>
auto make_scoped_handler(int signal_number, F&& callback) {
    return ScopedSignalHandler(signal_number, std::forward<F>(callback));
}

} // namespace sig
