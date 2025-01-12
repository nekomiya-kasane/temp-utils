#include "signal_slot.hpp"

namespace signal_slot {

Connection::Connection(std::weak_ptr<SignalBase> signal, std::weak_ptr<SlotBase> slot,
                     ConnectionType type, Priority priority)
    : signal_(std::move(signal))
    , slot_(std::move(slot))
    , type_(type)
    , priority_(priority) {
}

void Connection::disconnect() {
    if (auto signal = signal_.lock()) {
        if (auto slot = slot_.lock()) {
            signal->disconnect(slot);
        }
    }
}

bool Connection::isValid() const {
    return !signal_.expired() && !slot_.expired();
}

void Connection::setEnabled(bool enabled) {
    enabled_ = enabled;
}

bool Connection::isEnabled() const {
    return enabled_;
}

void SignalBase::disconnect(const std::shared_ptr<SlotBase>& slot) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    slots_.erase(
        std::remove_if(slots_.begin(), slots_.end(),
            [&slot](const SlotEntry& entry) {
                if (auto s = entry.slot.lock()) {
                    return s == slot;
                }
                return false;
            }),
        slots_.end()
    );
    
    if (auto s = slot) {
        std::lock_guard<std::mutex> slotLock(s->mutex_);
        s->connections_.erase(
            std::remove_if(s->connections_.begin(), s->connections_.end(),
                [this](const Connection& conn) {
                    return !conn.signal_.expired() && 
                           conn.signal_.lock().get() == this;
                }),
            s->connections_.end()
        );
    }
}

void SignalBase::disconnectAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& entry : slots_) {
        if (auto slot = entry.slot.lock()) {
            std::lock_guard<std::mutex> slotLock(slot->mutex_);
            slot->connections_.erase(
                std::remove_if(slot->connections_.begin(), slot->connections_.end(),
                    [this](const Connection& conn) {
                        return !conn.signal_.expired() && 
                               conn.signal_.lock().get() == this;
                    }),
                slot->connections_.end()
            );
        }
    }
    
    slots_.clear();
}

EventLoop::EventLoop() : mainThreadId_(std::this_thread::get_id()) {
}

EventLoop::~EventLoop() {
    stop();
}

void EventLoop::start() {
    if (!running_) {
        running_ = true;
        eventThread_ = std::thread(&EventLoop::processEvents, this);
    }
}

void EventLoop::stop() {
    if (running_) {
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            running_ = false;
        }
        condition_.notify_one();
        if (eventThread_.joinable()) {
            eventThread_.join();
        }
    }
}

void EventLoop::postEvent(std::function<void()> event) {
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        eventQueue_.push(std::move(event));
    }
    condition_.notify_one();
}

void EventLoop::processEvents() {
    while (running_) {
        std::function<void()> event;
        
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            condition_.wait(lock, [this] {
                return !running_ || !eventQueue_.empty();
            });
            
            if (!running_ && eventQueue_.empty()) {
                break;
            }
            
            if (!eventQueue_.empty()) {
                event = std::move(eventQueue_.front());
                eventQueue_.pop();
            }
        }
        
        if (event) {
            try {
                event();
            } catch (const std::exception& e) {
                // 处理异常
                std::cerr << "Exception in event handler: " << e.what() << std::endl;
            }
        }
    }
}

} // namespace signal_slot
