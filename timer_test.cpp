#include "timer.h"
#include <gtest/gtest.h>
#include <atomic>
#include <thread>
#include <vector>

class TimerTest : public testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TimerTest, BasicTimer) {
    Timer<> timer;
    std::atomic<int> counter{0};
    
    timer.start();
    
    // Test setTimeout
    timer.setTimeout(std::chrono::milliseconds(100), [&counter]() {
        counter++;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(counter, 1);
    
    // Test setInterval
    timer.setInterval(std::chrono::milliseconds(50), [&counter]() {
        counter++;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(175));
    EXPECT_GE(counter, 4);  // Should trigger at least 3 more times
    
    timer.stop();
}

TEST_F(TimerTest, TimerPrecisions) {
    std::atomic<int> low_counter{0};
    std::atomic<int> medium_counter{0};
    std::atomic<int> high_counter{0};
    
    LowPrecisionTimer low_timer;
    MediumPrecisionTimer medium_timer;
    HighPrecisionTimer high_timer;
    
    low_timer.start();
    medium_timer.start();
    high_timer.start();
    
    // Set intervals with different precisions
    low_timer.setInterval(std::chrono::milliseconds(100), [&low_counter]() {
        low_counter++;
    });
    
    medium_timer.setInterval(std::chrono::milliseconds(50), [&medium_counter]() {
        medium_counter++;
    });
    
    high_timer.setInterval(std::chrono::milliseconds(10), [&high_counter]() {
        high_counter++;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    low_timer.stop();
    medium_timer.stop();
    high_timer.stop();
    
    // Check counters
    EXPECT_GE(low_counter, 4);     // Should trigger at least 4 times
    EXPECT_GE(medium_counter, 9);   // Should trigger at least 9 times
    EXPECT_GE(high_counter, 45);    // Should trigger at least 45 times
}

TEST_F(TimerTest, ClearTimer) {
    Timer<> timer;
    std::atomic<int> counter{0};
    
    timer.start();
    
    timer.setInterval(std::chrono::milliseconds(50), [&counter]() {
        counter++;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(175));
    int first_count = counter;
    EXPECT_GE(first_count, 3);
    
    timer.clear();
    counter = 0;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(175));
    EXPECT_EQ(counter, 0);
    
    timer.stop();
}

TEST_F(TimerTest, ThreadSafety) {
    ThreadSafeTimer<> timer;
    std::atomic<int> counter{0};
    
    timer.start();
    
    // Create multiple threads that add timers
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&timer, &counter, i]() {
            timer.setTimeout(std::chrono::milliseconds(50 + i * 10), [&counter]() {
                counter++;
            });
        });
    }
    
    // Join all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(counter, 10);
    
    timer.stop();
}

TEST_F(TimerTest, HighPrecisionCallback) {
    HighPrecisionTimer timer;
    std::atomic<int> counter{0};
    std::atomic<bool> in_callback{false};
    std::atomic<int> concurrent_callbacks{0};
    
    timer.start();
    
    // Add multiple high-frequency timers
    for (int i = 0; i < 5; ++i) {
        timer.setInterval(std::chrono::milliseconds(1), [&]() {
            if (in_callback) {
                concurrent_callbacks++;
            }
            in_callback = true;
            std::this_thread::sleep_for(std::chrono::microseconds(500));
            counter++;
            in_callback = false;
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    timer.stop();
    
    // Check that callbacks were executed concurrently
    EXPECT_GT(concurrent_callbacks, 0);
    EXPECT_GT(counter, 100);  // Should have triggered many times
}

TEST_F(TimerTest, TimerAccuracy) {
    Timer<TimerPrecision::High> timer;
    std::atomic<int64_t> last_time{0};
    std::atomic<int> interval_count{0};
    std::vector<int64_t> intervals;
    std::mutex intervals_mutex;
    
    timer.start();
    
    // Measure actual intervals between callbacks
    timer.setInterval(std::chrono::milliseconds(10), [&]() {
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        auto last = last_time.exchange(now);
        
        if (last != 0) {
            std::lock_guard<std::mutex> lock(intervals_mutex);
            intervals.push_back(now - last);
        }
        
        interval_count++;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    timer.stop();
    
    // Calculate statistics
    double sum = 0;
    double squared_sum = 0;
    
    for (auto interval : intervals) {
        sum += interval;
        squared_sum += interval * interval;
    }
    
    double mean = sum / intervals.size();
    double variance = (squared_sum / intervals.size()) - (mean * mean);
    double std_dev = std::sqrt(variance);
    
    // Check timing accuracy
    EXPECT_GT(interval_count, 45);  // Should have at least 45 intervals
    EXPECT_LT(std_dev / mean, 0.1); // Standard deviation should be less than 10% of mean
}
