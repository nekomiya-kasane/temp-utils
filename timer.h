#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>

// Timer precision requirements
enum class TimerPrecision {
    Low,     // Uses system timer, suitable for longer intervals (>100ms)
    Medium,  // Uses high resolution timer, suitable for medium intervals (10-100ms)
    High     // Uses high precision timer, suitable for short intervals (<10ms)
};

// Timer event with callback
struct TimerEvent {
    std::chrono::steady_clock::time_point next_trigger;
    std::chrono::nanoseconds interval;
    std::function<void()> callback;
    bool repeat;
    bool active;

    // Compare operators for priority queue
    bool operator>(const TimerEvent& other) const {
        return next_trigger > other.next_trigger;
    }
};

// Thread-safe timer implementation
template<TimerPrecision Precision = TimerPrecision::Medium>
class Timer {
public:
    Timer() : running_(false) {}

    ~Timer() {
        stop();
    }

    // Start the timer thread
    void start() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_) {
            running_ = true;
            thread_ = std::thread(&Timer::run, this);
        }
    }

    // Stop the timer thread
    void stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_) return;
            running_ = false;
        }
        cv_.notify_one();
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    // Add a one-shot timer
    template<typename Rep, typename Period, typename Callback>
    void setTimeout(const std::chrono::duration<Rep, Period>& delay, Callback&& callback) {
        auto event = std::make_shared<TimerEvent>();
        event->next_trigger = std::chrono::steady_clock::now() + delay;
        event->interval = std::chrono::duration_cast<std::chrono::nanoseconds>(delay);
        event->callback = std::forward<Callback>(callback);
        event->repeat = false;
        event->active = true;

        addEvent(event);
    }

    // Add a repeating timer
    template<typename Rep, typename Period, typename Callback>
    void setInterval(const std::chrono::duration<Rep, Period>& interval, Callback&& callback) {
        auto event = std::make_shared<TimerEvent>();
        event->next_trigger = std::chrono::steady_clock::now() + interval;
        event->interval = std::chrono::duration_cast<std::chrono::nanoseconds>(interval);
        event->callback = std::forward<Callback>(callback);
        event->repeat = true;
        event->active = true;

        addEvent(event);
    }

    // Clear all timers
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!events_.empty()) {
            events_.pop();
        }
    }

private:
    void run() {
        while (running_) {
            std::unique_lock<std::mutex> lock(mutex_);

            if (events_.empty()) {
                cv_.wait(lock, [this] { return !running_ || !events_.empty(); });
                if (!running_) break;
            }

            auto now = std::chrono::steady_clock::now();
            auto& event = events_.top();

            if (event->next_trigger <= now) {
                // Execute the callback
                auto callback = event->callback;
                bool repeat = event->repeat;
                
                if (repeat) {
                    // Reschedule the event
                    event->next_trigger += event->interval;
                    auto current = events_.top();
                    events_.pop();
                    events_.push(current);
                } else {
                    events_.pop();
                }

                // Release the lock before callback execution
                lock.unlock();
                
                if (callback) {
                    if constexpr (Precision == TimerPrecision::High) {
                        // For high precision timers, use a separate thread for callback
                        std::thread([callback = std::move(callback)]() {
                            callback();
                        }).detach();
                    } else {
                        callback();
                    }
                }
            } else {
                if constexpr (Precision == TimerPrecision::Low) {
                    // For low precision, use longer sleep intervals
                    cv_.wait_for(lock, std::chrono::milliseconds(10));
                } else if constexpr (Precision == TimerPrecision::Medium) {
                    // For medium precision, use shorter sleep intervals
                    cv_.wait_for(lock, std::chrono::milliseconds(1));
                } else {
                    // For high precision, use minimal sleep
                    cv_.wait_for(lock, std::chrono::microseconds(100));
                }
            }
        }
    }

    void addEvent(std::shared_ptr<TimerEvent> event) {
        std::lock_guard<std::mutex> lock(mutex_);
        events_.push(std::move(event));
        cv_.notify_one();
    }

private:
    std::atomic<bool> running_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::thread thread_;
    std::priority_queue<
        std::shared_ptr<TimerEvent>,
        std::vector<std::shared_ptr<TimerEvent>>,
        std::greater<>
    > events_;
};

// Thread-safe timer with additional features
template<TimerPrecision Precision = TimerPrecision::Medium>
class ThreadSafeTimer : public Timer<Precision> {
public:
    ThreadSafeTimer() = default;
    ~ThreadSafeTimer() = default;

    // Prevent copying
    ThreadSafeTimer(const ThreadSafeTimer&) = delete;
    ThreadSafeTimer& operator=(const ThreadSafeTimer&) = delete;

    // Allow moving
    ThreadSafeTimer(ThreadSafeTimer&&) = default;
    ThreadSafeTimer& operator=(ThreadSafeTimer&&) = default;
};

// Convenience type aliases
using LowPrecisionTimer = Timer<TimerPrecision::Low>;
using MediumPrecisionTimer = Timer<TimerPrecision::Medium>;
using HighPrecisionTimer = Timer<TimerPrecision::High>;

using ThreadSafeLowPrecisionTimer = ThreadSafeTimer<TimerPrecision::Low>;
using ThreadSafeMediumPrecisionTimer = ThreadSafeTimer<TimerPrecision::Medium>;
using ThreadSafeHighPrecisionTimer = ThreadSafeTimer<TimerPrecision::High>;
