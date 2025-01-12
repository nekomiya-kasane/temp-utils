#include <gtest/gtest.h>
#include "signal_handler.hpp"
#include <thread>
#include <atomic>
#include <chrono>

class SignalHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        sig::SignalHandler::instance().reset_all();
    }

    void TearDown() override {
        sig::SignalHandler::instance().reset_all();
    }
};

TEST_F(SignalHandlerTest, BasicSignalHandling) {
    std::atomic<bool> signal_received{false};
    
    // Register handler for SIGTERM
    ASSERT_TRUE(sig::SignalHandler::instance().register_handler(
        SIGTERM,
        [&](int) { signal_received = true; }
    ));
    
    // Raise signal
    raise(SIGTERM);
    
    // Give some time for the signal to be processed
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_TRUE(signal_received);
}

TEST_F(SignalHandlerTest, MultipleSignals) {
    std::atomic<int> signal_count{0};
    
    // Register handler for multiple signals
    std::vector<int> signals = {SIGTERM, SIGINT};
    ASSERT_TRUE(sig::SignalHandler::instance().register_handler(
        signals,
        [&](int) { ++signal_count; }
    ));
    
    // Raise both signals
    raise(SIGTERM);
    raise(SIGINT);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_EQ(signal_count, 2);
}

TEST_F(SignalHandlerTest, SignalBlocking) {
    std::atomic<bool> signal_received{false};
    
    // Register handler
    sig::SignalHandler::instance().register_handler(
        SIGTERM,
        [&](int) { signal_received = true; }
    );
    
    {
        // Block SIGTERM
        sig::SignalHandler::SignalBlocker blocker({SIGTERM});
        
        // Signal should be blocked
        raise(SIGTERM);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_FALSE(signal_received);
    }
    
    // Signal should be processed now
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_TRUE(signal_received);
}

TEST_F(SignalHandlerTest, ScopedHandler) {
    std::atomic<bool> signal_received{false};
    
    {
        auto handler = sig::make_scoped_handler(
            SIGTERM,
            [&](int) { signal_received = true; }
        );
        
        raise(SIGTERM);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        EXPECT_TRUE(signal_received);
    }
    
    // Reset signal_received
    signal_received = false;
    
    // Handler should be unregistered now
    raise(SIGTERM);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(signal_received);
}

TEST_F(SignalHandlerTest, SignalInfo) {
    auto info = sig::SignalHandler::get_signal_info(SIGTERM);
    EXPECT_EQ(info.signal_number, SIGTERM);
    EXPECT_EQ(info.name, "SIGTERM");
    EXPECT_FALSE(info.is_default_fatal);
}

TEST_F(SignalHandlerTest, HandlerLocation) {
    const auto loc = std::source_location::current();
    sig::SignalHandler::instance().register_handler(SIGTERM, [](int){}, loc);
    
    auto handler_loc = sig::SignalHandler::instance().get_handler_location(SIGTERM);
    EXPECT_EQ(handler_loc.line(), loc.line());
    EXPECT_EQ(handler_loc.file_name(), loc.file_name());
}

TEST_F(SignalHandlerTest, ConcurrentSignals) {
    std::atomic<int> signal_count{0};
    
    // Register handler
    sig::SignalHandler::instance().register_handler(
        SIGTERM,
        [&](int) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            ++signal_count;
        }
    );
    
    // Raise signals from multiple threads
    constexpr int num_threads = 10;
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([]{
            raise(SIGTERM);
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    EXPECT_EQ(signal_count, num_threads);
}

TEST_F(SignalHandlerTest, ExceptionInHandler) {
    bool exception_caught = false;
    
    // Register handler that throws
    sig::SignalHandler::instance().register_handler(
        SIGTERM,
        [](int) { throw std::runtime_error("Test exception"); }
    );
    
    try {
        raise(SIGTERM);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } catch (const std::runtime_error&) {
        exception_caught = true;
    }
    
    EXPECT_TRUE(exception_caught);
    EXPECT_FALSE(sig::SignalHandler::instance().is_handling_signal(SIGTERM));
}

TEST_F(SignalHandlerTest, RegisterUnregister) {
    std::atomic<bool> signal_received{false};
    
    // Register handler
    ASSERT_TRUE(sig::SignalHandler::instance().register_handler(
        SIGTERM,
        [&](int) { signal_received = true; }
    ));
    
    // Unregister handler
    ASSERT_TRUE(sig::SignalHandler::instance().unregister_handler(SIGTERM));
    
    // Signal should not be handled
    raise(SIGTERM);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_FALSE(signal_received);
}

TEST_F(SignalHandlerTest, GetRegisteredSignals) {
    // Register handlers for multiple signals
    sig::SignalHandler::instance().register_handler(SIGTERM, [](int){});
    sig::SignalHandler::instance().register_handler(SIGINT, [](int){});
    
    auto signals = sig::SignalHandler::instance().get_registered_signals();
    EXPECT_EQ(signals.size(), 2);
    EXPECT_TRUE(std::find(signals.begin(), signals.end(), SIGTERM) != signals.end());
    EXPECT_TRUE(std::find(signals.begin(), signals.end(), SIGINT) != signals.end());
}
