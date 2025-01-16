#include "signal.h"
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <vector>
#include <functional>


// 基本功能测试
TEST(SignalTest, BasicFunctionality) {
  Signal<int(int)> signal;
  int result = 0;

  auto id = signal.connect([](int x) { return x * 2; });
  EXPECT_TRUE(signal.isConnected(id));
  
  auto results = signal.emit(21);
  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0], 42);

  signal.disconnect(id);
  EXPECT_FALSE(signal.isConnected(id));
}

// 多个槽函数测试
TEST(SignalTest, MultipleSlots) {
  Signal<void(int)> signal;
  int sum = 0;

  signal.connect([&](int x) { sum += x; });
  signal.connect([&](int x) { sum += x * 2; });
  signal.connect([&](int x) { sum += x * 3; });

  signal.emit(1);  // sum += 1 + 2 + 3 = 6
  EXPECT_EQ(sum, 6);
}

//// 优先级测试
//TEST(SignalTest, Priority) {
//  Signal<void(std::string&)> signal;
//  std::string result;
//
//  signal.connect([&](std::string& s) { s += "3"; }, ConnectionType::Direct, -1);
//  signal.connect([&](std::string& s) { s += "1"; }, ConnectionType::Direct, 1);
//  signal.connect([&](std::string& s) { s += "2"; }, ConnectionType::Direct, 0);
//
//  signal.emit(result);
//  EXPECT_EQ(result, "123");
//}

// 线程安全测试
TEST(SignalTest, ThreadSafety) {
  Signal<void(int), ThreadSafety::Exclusive> signal;
  std::atomic<int> counter{0};

  // 创建多个线程同时发射信号
  std::vector<std::thread> threads;
  for (int i = 0; i < 10; ++i) {
    signal.connect([&](int x) { counter += x; });
  }

  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&] {
      for (int j = 0; j < 100; ++j) {
        signal.emit(1);
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  EXPECT_EQ(counter, 10 * 10 * 100);  // 10个槽 * 10个线程 * 100次发射
}

//// 队列连接测试
//TEST(SignalTest, QueuedConnections) {
//  Signal<int(int)> signal;
//  std::vector<int> results;
//
//  signal.connect([&](int x) { return x + 1; }, ConnectionType::Queued, 0);
//  signal.connect([&](int x) { return x + 2; }, ConnectionType::Queued, 1);
//  signal.connect([&](int x) { return x + 3; }, ConnectionType::BlockingQueued, -1);
//
//  signal.emit(10);
//  EXPECT_TRUE(signal.hasPendingTasks());
//  
//  signal.processQueue();
//  EXPECT_FALSE(signal.hasPendingTasks());
//}

// 作用域连接测试
TEST(SignalTest, ScopedConnection) {
  Signal<void()> signal;
  int counter = 0;

  {
    ScopedConnection conn(&signal, signal.connect([&] { ++counter; }));
    signal.emit();
    EXPECT_EQ(counter, 1);
  }  // conn 超出作用域，自动断开连接

  signal.emit();
  EXPECT_EQ(counter, 1);  // 计数器不再增加
}

// 禁用/启用连接测试
TEST(SignalTest, EnableDisableConnections) {
  Signal<void(int)> signal;
  int sum = 0;

  auto id = signal.connect([&](int x) { sum += x; });
  
  signal.emit(1);
  EXPECT_EQ(sum, 1);

  signal.setEnabled(id, false);
  signal.emit(1);
  EXPECT_EQ(sum, 1);  // sum 没有改变

  signal.setEnabled(id, true);
  signal.emit(1);
  EXPECT_EQ(sum, 2);
}

// 返回值测试
TEST(SignalTest, ReturnValues) {
  Signal<std::string(int)> signal;

  signal.connect([](int x) { return "A" + std::to_string(x); });
  signal.connect([](int x) { return "B" + std::to_string(x); });

  auto results = signal.emit(42);
  EXPECT_EQ(results.size(), 2);
  EXPECT_EQ(results[0], "A42");
  EXPECT_EQ(results[1], "B42");
}

// 用于测试成员函数连接的类
class TestObject : public std::enable_shared_from_this<TestObject> {
public:
  int value = 0;
  
  int increment(int x) {
    value += x;
    return value;
  }
  
  void reset() {
    value = 0;
  }
  
  int getValue() const {
    return value;
  }
};

// 测试成员函数连接
TEST(SignalTest, MemberFunctionConnection) {
  Signal<int(int)> signal;
  auto obj = std::make_shared<TestObject>();
  
  auto id = signal.connect(obj.get(), &TestObject::increment);
  EXPECT_TRUE(signal.isConnected(id));
  
  auto results = signal.emit(5);
  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0], 5);
  EXPECT_EQ(obj->value, 5);
}

// 测试自动断开连接
TEST(SignalTest, AutoDisconnect) {
  Signal<void(int)> signal;
  
  {
    auto obj = std::make_shared<TestObject>();
    ConnectionOptions opts;
    opts.autoDisconnect = true;
    
    signal.connect(obj.get(), &TestObject::increment, opts);
    signal.emit(5);
    EXPECT_EQ(obj->value, 5);
  } // obj 超出作用域
  
  signal.cleanup();  // 清理失效的连接
  EXPECT_EQ(signal.connectionCount(), 0);
}

// 测试特定连接的信号发射
TEST(SignalTest, EmitToSpecificConnection) {
  Signal<int(int)> signal;
  
  auto id1 = signal.connect([](int x) { return x * 2; });
  auto id2 = signal.connect([](int x) { return x * 3; });
  
  //auto result1 = signal.emitTo(id1, 5);
  //EXPECT_TRUE(result1.has_value());
  //EXPECT_EQ(*result1, 10);
  //
  //auto result2 = signal.emitTo(id2, 5);
  //EXPECT_TRUE(result2.has_value());
  //EXPECT_EQ(*result2, 15);
}

// 测试暂停/恢复连接
TEST(SignalTest, PauseResumeConnections) {
  Signal<void(int)> signal;
  int sum = 0;
  
  signal.connect([&](int x) { sum += x; });
  
  signal.emit(1);
  EXPECT_EQ(sum, 1);
  
  signal.pauseAll();
  signal.emit(1);
  EXPECT_EQ(sum, 1);  // sum 没有改变
  
  signal.resumeAll();
  signal.emit(1);
  EXPECT_EQ(sum, 2);
}

// 测试批量连接
TEST(SignalTest, BatchConnections) {
  Signal<int(int)> signal;
  std::vector<std::function<int(int)>> slots = {
    [](int x) { return x + 1; },
    [](int x) { return x + 2; },
    [](int x) { return x + 3; }
  };
  
  auto ids = signal.connectAll(slots);
  EXPECT_EQ(ids.size(), 3);
  
  auto results = signal.emit(1);
  EXPECT_EQ(results.size(), 3);
  EXPECT_EQ(results[0], 2);
  EXPECT_EQ(results[1], 3);
  EXPECT_EQ(results[2], 4);
  
  signal.disconnectAll(ids);
  EXPECT_EQ(signal.connectionCount(), 0);
}

// 测试队列刷新
TEST(SignalTest, QueueFlush) {
  Signal<int(int)> signal;
  std::vector<int> results;
  
  ConnectionOptions opts;
  opts.type = ConnectionType::Queued;
  
  signal.connect([&](int x) {
    results.push_back(x * 2);
    return x * 2;
  }, opts);
  
  signal.emit(1);
  signal.emit(2);
  signal.emit(3);
  
  EXPECT_TRUE(signal.hasPendingTasks());
  signal.flush();  // 处理所有待处理的任务
  EXPECT_FALSE(signal.hasPendingTasks());
  
  EXPECT_EQ(results.size(), 3);
  EXPECT_EQ(results[0], 2);
  EXPECT_EQ(results[1], 4);
  EXPECT_EQ(results[2], 6);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
