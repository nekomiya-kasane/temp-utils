#include <gtest/gtest.h>
#include "basic_signal.h"

class TestReceiver {
public:
    void slot1(int value) { lastValue = value; callCount1++; }
    void slot2(int value) { lastValue = value * 2; callCount2++; }
    
    void beforeSlot() { beforeCalled = true; }
    void afterSlot() { afterCalled = true; }
    
    int lastValue = 0;
    int callCount1 = 0;
    int callCount2 = 0;
    bool beforeCalled = false;
    bool afterCalled = false;
};

class SignalTest : public testing::Test {
protected:
    void SetUp() override {
        receiver = std::make_unique<TestReceiver>();
    }

    void TearDown() override {
        receiver.reset();
    }

    std::unique_ptr<TestReceiver> receiver;
};

TEST_F(SignalTest, BasicConnectionAndEmission) {
    Signal<void(int)> signal;
    auto conn = signal.connect(receiver.get(), &TestReceiver::slot1);
    signal.emit(42);
    EXPECT_EQ(receiver->lastValue, 42);
    EXPECT_EQ(receiver->callCount1, 1);
}

TEST_F(SignalTest, MultipleSlots) {
    Signal<void(int)> signal;
    auto conn1 = signal.connect(receiver.get(), &TestReceiver::slot1);
    auto conn2 = signal.connect(receiver.get(), &TestReceiver::slot2);
    
    signal(10);
    EXPECT_EQ(receiver->callCount1, 1);
    EXPECT_EQ(receiver->callCount2, 1);
    EXPECT_EQ(receiver->lastValue, 20); // slot2 executes last and multiplies by 2
}

TEST_F(SignalTest, ConnectionManagement) {
    Signal<void(int)> signal;
    auto conn = signal.connect(receiver.get(), &TestReceiver::slot1);
    
    signal(1);
    EXPECT_EQ(receiver->callCount1, 1);
    
    conn.setEnabled(false);
    signal(2);
    EXPECT_EQ(receiver->callCount1, 1); // Should not increment
    
    conn.setEnabled(true);
    signal(3);
    EXPECT_EQ(receiver->callCount1, 2);
    
    conn.disconnect();
    signal(4);
    EXPECT_EQ(receiver->callCount1, 2); // Should not increment
    EXPECT_FALSE(conn.isValid());
}

TEST_F(SignalTest, PriorityBasedOrdering) {
    Signal<void(int)> signal;
    ConnectionProperties highPriority;
    highPriority.priority = 1;
    
    ConnectionProperties lowPriority;
    lowPriority.priority = -1;
    
    auto conn1 = signal.connect(receiver.get(), &TestReceiver::slot1, lowPriority);
    auto conn2 = signal.connect(receiver.get(), &TestReceiver::slot2, highPriority);
    
    signal(5);
    EXPECT_EQ(receiver->lastValue, 5); // slot1 should execute last due to lower priority
}

TEST_F(SignalTest, BeforeAfterHooks) {
    Signal<void(int)> signal;
    auto conn = signal.connect(receiver.get(), &TestReceiver::slot1);
    
    signal(6);
    EXPECT_TRUE(receiver->beforeCalled);
    EXPECT_TRUE(receiver->afterCalled);
}

TEST_F(SignalTest, LambdaSlots) {
    Signal<void(int)> signal;
    int capturedValue = 0;
    
    auto conn = signal.connect(nullptr, [&](int value) { capturedValue = value; });
    signal(42);
    EXPECT_EQ(capturedValue, 42);
}

TEST_F(SignalTest, ConnectionLifetime) {
    Signal<void(int)> signal;
    {
        auto conn = signal.connect(receiver.get(), &TestReceiver::slot1);
        signal(1);
        EXPECT_EQ(receiver->callCount1, 1);
    } // conn goes out of scope here
    
    signal(2);
    EXPECT_EQ(receiver->callCount1, 1); // Should not increment
}