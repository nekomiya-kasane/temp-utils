#include "signal_handler.hpp"
#include <stdexcept>
#include <system_error>
#include <array>

namespace sig {

// Static member initialization
SignalHandler* SignalHandler::instance_ = nullptr;
std::mutex SignalHandler::instance_mutex_;

// Signal information table
static const std::array<SignalInfo, 6> signal_info_table = {{
    {SIGABRT, "SIGABRT", "Abnormal termination", true},
    {SIGFPE, "SIGFPE", "Floating-point exception", true},
    {SIGILL, "SIGILL", "Illegal instruction", true},
    {SIGINT, "SIGINT", "Interactive attention signal", false},
    {SIGSEGV, "SIGSEGV", "Segmentation violation", true},
    {SIGTERM, "SIGTERM", "Termination request", false}
}};

SignalHandler& SignalHandler::instance() {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (!instance_) {
        instance_ = new SignalHandler();
    }
    return *instance_;
}

SignalHandler::SignalHandler() {
    // Initialize handling_signals_ for all known signals
    for (const auto& info : signal_info_table) {
        handling_signals_[info.signal_number].store(false);
    }
}

SignalHandler::~SignalHandler() {
    reset_all();
}

bool SignalHandler::register_handler_impl(int signal_number, SignalCallback callback,
                                        const std::source_location& loc) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Save original handler if not already saved
    if (original_handlers_.find(signal_number) == original_handlers_.end()) {
        struct sigaction old_action;
        if (sigaction(signal_number, nullptr, &old_action) == 0) {
            original_handlers_[signal_number] = old_action;
        }
    }

    // Set up new signal handler
    struct sigaction new_action;
    new_action.sa_handler = &SignalHandler::signal_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = SA_RESTART;

    if (sigaction(signal_number, &new_action, nullptr) != 0) {
        throw std::system_error(errno, std::system_category(),
                              "Failed to register signal handler");
    }

    handlers_[signal_number] = std::move(callback);
    handler_locations_[signal_number] = loc;
    return true;
}

void SignalHandler::signal_handler(int signal) {
    instance().handle_signal(signal);
}

void SignalHandler::handle_signal(int signal) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = handlers_.find(signal);
    if (it != handlers_.end()) {
        handling_signals_[signal].store(true);
        try {
            it->second(signal);
        } catch (...) {
            // Log or handle exception in signal handler
            handling_signals_[signal].store(false);
            throw;
        }
        handling_signals_[signal].store(false);
    }
}

bool SignalHandler::unregister_handler(int signal_number) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = original_handlers_.find(signal_number);
    if (it != original_handlers_.end()) {
        if (sigaction(signal_number, &it->second, nullptr) != 0) {
            return false;
        }
        handlers_.erase(signal_number);
        handler_locations_.erase(signal_number);
        original_handlers_.erase(it);
        return true;
    }
    return false;
}

bool SignalHandler::is_handling_signal(int signal_number) const {
    auto it = handling_signals_.find(signal_number);
    return it != handling_signals_.end() && it->second.load();
}

SignalInfo SignalHandler::get_signal_info(int signal_number) {
    for (const auto& info : signal_info_table) {
        if (info.signal_number == signal_number) {
            return info;
        }
    }
    throw std::invalid_argument("Unknown signal number");
}

bool SignalHandler::reset_to_default(int signal_number) {
    return unregister_handler(signal_number);
}

void SignalHandler::reset_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& [signal, _] : handlers_) {
        reset_to_default(signal);
    }
    
    handlers_.clear();
    handler_locations_.clear();
    original_handlers_.clear();
}

bool SignalHandler::is_signal_blocked(int signal_number) const {
    sigset_t current_mask;
    if (pthread_sigmask(SIG_BLOCK, nullptr, &current_mask) != 0) {
        throw std::system_error(errno, std::system_category(),
                              "Failed to get signal mask");
    }
    return sigismember(&current_mask, signal_number) == 1;
}

std::vector<int> SignalHandler::get_registered_signals() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<int> signals;
    signals.reserve(handlers_.size());
    for (const auto& [signal, _] : handlers_) {
        signals.push_back(signal);
    }
    return signals;
}

std::source_location SignalHandler::get_handler_location(int signal_number) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = handler_locations_.find(signal_number);
    if (it != handler_locations_.end()) {
        return it->second;
    }
    return std::source_location::current();
}

// SignalBlocker implementation
SignalHandler::SignalBlocker::SignalBlocker(const std::vector<int>& signals) {
    sigset_t new_mask;
    sigemptyset(&new_mask);
    for (int sig : signals) {
        sigaddset(&new_mask, sig);
    }
    if (pthread_sigmask(SIG_BLOCK, &new_mask, &old_mask_) != 0) {
        throw std::system_error(errno, std::system_category(),
                              "Failed to block signals");
    }
}

SignalHandler::SignalBlocker::~SignalBlocker() {
    pthread_sigmask(SIG_SETMASK, &old_mask_, nullptr);
}

} // namespace sig
