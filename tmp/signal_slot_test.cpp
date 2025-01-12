#include "signal_slot.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <numeric>

// 测试类
class Button {
public:
    signal_slot::Signal<> clicked;
    signal_slot::Signal<int, int> mouseMoved;
    signal_slot::Signal<std::string> textChanged;
    
    void click() {
        std::cout << "Button clicked\n";
        clicked.emit();
    }
    
    void mouseMove(int x, int y) {
        std::cout << "Mouse moved to (" << x << ", " << y << ")\n";
        mouseMoved.emit(x, y);
    }
    
    void setText(const std::string& text) {
        std::cout << "Setting text to: " << text << "\n";
        textChanged.emit(text);
    }
};

class Label {
public:
    explicit Label(const std::string& text = "") : text_(text) {}
    
    void setText(const std::string& text) {
        text_ = text;
        std::cout << "Label text changed to: " << text_ << "\n";
    }
    
    void onMouseMove(int x, int y) {
        std::cout << "Label received mouse move: (" << x << ", " << y << ")\n";
    }
    
private:
    std::string text_;
};

// 全局函数
void globalClickHandler() {
    std::cout << "Global click handler called\n";
}

void globalMouseHandler(int x, int y) {
    std::cout << "Global mouse handler called: (" << x << ", " << y << ")\n";
}

// Test class with return values
class Calculator {
public:
    signal_slot::SignalAll<int(int, int)> calculate;
    signal_slot::SignalFirst<std::string(int)> validate;
    signal_slot::SignalLast<double(double)> process;

    int add(int a, int b) {
        auto results = calculate.emit(a, b);
        return results.empty() ? 0 : results[0];
    }

    std::string validateNumber(int n) {
        auto results = validate.emit(n);
        return results.empty() ? "Invalid" : results[0];
    }

    double processValue(double v) {
        auto results = process.emit(v);
        return results.empty() ? v : results[0];
    }
};

// Test class with before/after hooks
class Handler {
public:
    void beforeSlot() {
        std::cout << "Before slot called\n";
    }

    void afterSlot() {
        std::cout << "After slot called\n";
    }

    int multiply(int a, int b) {
        std::cout << "Multiply called: " << a << " * " << b << "\n";
        return a * b;
    }
};

// Test basic signal functionality
void testBasicSignals() {
    std::cout << "\n=== Testing Basic Signals ===\n";
    
    Calculator calc;
    Handler handler;
    
    // Test member function with hooks
    calc.calculate.connect(&handler, &Handler::multiply);
    
    // Test lambda with return value
    calc.calculate.connect([](int a, int b) {
        std::cout << "Lambda add: " << a << " + " << b << "\n";
        return a + b;
    });
    
    // Test with different connection properties
    signal_slot::ConnectionProperties props;
    props.type = signal_slot::ConnectionProperties::Type::Direct;
    props.priority = signal_slot::ConnectionProperties::Priority::High;
    
    calc.calculate.connect([](int a, int b) {
        std::cout << "High priority subtract: " << a << " - " << b << "\n";
        return a - b;
    }, props);
    
    // Test emit and collect all results
    std::cout << "Testing calculate signal:\n";
    auto results = calc.calculate.emit(10, 5);
    std::cout << "Got " << results.size() << " results:\n";
    for (auto result : results) {
        std::cout << "Result: " << result << "\n";
    }
}

// Test return value collection strategies
void testReturnStrategies() {
    std::cout << "\n=== Testing Return Strategies ===\n";
    
    Calculator calc;
    
    // Test SignalFirst
    calc.validate.connect([](int n) -> std::string {
        return n > 0 ? "Positive" : "";
    });
    
    calc.validate.connect([](int n) -> std::string {
        return n % 2 == 0 ? "Even" : "";
    });
    
    std::cout << "Validation result for 42: " << calc.validateNumber(42) << "\n";
    
    // Test SignalLast
    calc.process.connect([](double v) {
        return v * 2;
    });
    
    calc.process.connect([](double v) {
        return v + 10;
    });
    
    std::cout << "Process result for 5.0: " << calc.processValue(5.0) << "\n";
}

// Test custom collectors
void testCustomCollectors() {
    std::cout << "\n=== Testing Custom Collectors ===\n";
    
    signal_slot::Signal<int(int)> signal;
    
    // Add some test slots
    signal.connect([](int n) { return n * 2; });
    signal.connect([](int n) { return n + 10; });
    signal.connect([](int n) { return -n; });
    
    // Test FirstValidCollector
    auto validCollector = signal_slot::FirstValidCollector<int>(
        [](int val) { return val > 0; }
    );
    
    auto validResult = signal.emit_with(5, validCollector);
    if (validResult) {
        std::cout << "First valid result: " << *validResult << "\n";
    }
    
    // Test ReduceCollector
    auto reduceCollector = signal_slot::ReduceCollector<int>(
        0,
        [](int a, int b) { return a + b; }
    );
    
    auto sum = signal.emit_with(5, reduceCollector);
    std::cout << "Reduced result: " << sum << "\n";
}

// Test async signals with return values
void testAsyncSignals() {
    std::cout << "\n=== Testing Async Signals ===\n";
    
    signal_slot::Signal<int(int)> signal;
    signal_slot::EventLoop::getInstance().start();
    
    signal_slot::ConnectionProperties asyncProps;
    asyncProps.type = signal_slot::ConnectionProperties::Type::Queued;
    
    // Test async slot with return value
    signal.connect([](int n) {
        std::cout << "Async handler in thread: " << std::this_thread::get_id() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return n * 2;
    }, asyncProps);
    
    // Test mixed sync and async slots
    signal.connect([](int n) {
        std::cout << "Sync handler in thread: " << std::this_thread::get_id() << "\n";
        return n + 10;
    });
    
    std::cout << "Main thread: " << std::this_thread::get_id() << "\n";
    auto results = signal.emit(5);
    
    std::cout << "Got " << results.size() << " results\n";
    for (auto result : results) {
        std::cout << "Result: " << result << "\n";
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    signal_slot::EventLoop::getInstance().stop();
}

// Test error handling
void testErrorHandling() {
    std::cout << "\n=== Testing Error Handling ===\n";
    
    signal_slot::Signal<int(int)> signal;
    
    // Test slot that throws
    signal.connect([](int n) -> int {
        throw std::runtime_error("Test error");
    });
    
    // Test slot that returns normally
    signal.connect([](int n) { return n * 2; });
    
    auto results = signal.emit(5);
    std::cout << "Got " << results.size() << " results despite error\n";
    for (auto result : results) {
        std::cout << "Result: " << result << "\n";
    }
}

void testAsyncSignals2() {
    std::cout << "\n=== Testing Async Signals ===\n";
    
    Button button;
    Label label;
    
    // 启动事件循环
    signal_slot::EventLoop::getInstance().start();
    
    // 测试异步连接
    button.clicked.connect([]() {
        std::cout << "Async handler in thread: " 
                  << std::this_thread::get_id() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }, signal_slot::ConnectionType::QueuedConnection);
    
    // 测试多个异步处理器
    button.mouseMoved.connect([](int x, int y) {
        std::cout << "Async mouse handler 1: (" << x << ", " << y << ") in thread: "
                  << std::this_thread::get_id() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }, signal_slot::ConnectionType::QueuedConnection);
    
    button.mouseMoved.connect([](int x, int y) {
        std::cout << "Async mouse handler 2: (" << x << ", " << y << ") in thread: "
                  << std::this_thread::get_id() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }, signal_slot::ConnectionType::QueuedConnection);
    
    // 测试混合同步和异步处理器
    button.textChanged.connect([](const std::string& text) {
        std::cout << "Sync text handler: " << text << " in thread: "
                  << std::this_thread::get_id() << "\n";
    }, signal_slot::ConnectionType::DirectConnection);
    
    button.textChanged.connect([](const std::string& text) {
        std::cout << "Async text handler: " << text << " in thread: "
                  << std::this_thread::get_id() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }, signal_slot::ConnectionType::QueuedConnection);
    
    // 发射信号
    std::cout << "Main thread: " << std::this_thread::get_id() << "\n";
    
    button.click();
    button.mouseMove(100, 200);
    button.setText("Async Test");
    
    // 等待异步事件处理完成
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 停止事件循环
    signal_slot::EventLoop::getInstance().stop();
}

int main() {
    Button button;
    Label label("Initial Text");
    
    // 测试1: 连接成员函数
    button.textChanged.connect(std::bind(&Label::setText, &label, std::placeholders::_1));
    
    // 测试2: 连接lambda函数
    auto conn1 = button.clicked.connect([]() {
        std::cout << "Lambda click handler called\n";
    });
    
    // 测试3: 连接全局函数
    button.clicked.connect(globalClickHandler);
    
    // 测试4: 连接多个参数的信号
    button.mouseMoved.connect(&globalMouseHandler);
    button.mouseMoved.connect(std::bind(&Label::onMouseMove, &label, 
                                      std::placeholders::_1, 
                                      std::placeholders::_2));
    
    // 测试5: 不同优先级的连接
    button.clicked.connect([]() {
        std::cout << "High priority handler\n";
    }, signal_slot::ConnectionType::DirectConnection, signal_slot::Priority::High);
    
    button.clicked.connect([]() {
        std::cout << "Low priority handler\n";
    }, signal_slot::ConnectionType::DirectConnection, signal_slot::Priority::Low);
    
    // 测试信号发射
    std::cout << "\nTesting button click:\n";
    button.click();
    
    std::cout << "\nTesting mouse move:\n";
    button.mouseMove(100, 200);
    
    std::cout << "\nTesting text change:\n";
    button.setText("New Text");
    
    // 测试6: 断开连接
    std::cout << "\nTesting disconnect:\n";
    conn1.disconnect();
    button.click();  // Lambda处理器不会被调用
    
    // 测试7: 禁用/启用连接
    auto conn2 = button.clicked.connect([]() {
        std::cout << "Temporarily disabled handler\n";
    });
    
    std::cout << "\nTesting disabled connection:\n";
    conn2.setEnabled(false);
    button.click();
    
    std::cout << "\nTesting re-enabled connection:\n";
    conn2.setEnabled(true);
    button.click();
    
    // 测试8: 断开所有连接
    std::cout << "\nTesting disconnect all:\n";
    button.clicked.disconnectAll();
    button.click();  // 没有处理器会被调用
    
    testBasicSignals();
    testReturnStrategies();
    testCustomCollectors();
    testAsyncSignals();
    testErrorHandling();
    testAsyncSignals2();
    
    return 0;
}
