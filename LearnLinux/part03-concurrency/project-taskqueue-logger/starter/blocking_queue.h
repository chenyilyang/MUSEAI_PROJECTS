#pragma once
// ============================================================================
// 【骨架】BlockingQueue<T> —— 能编译、单线程能用，但并发语义未完成。
// 与任务书定型接口的差距（TODO）：
//   1. pop 在队列为空时应【阻塞等待】，骨架直接返回 false —— 会把"暂时没货"
//      误报成"已关闭且取空"，消费者会提前收工、丢数据。
//   2. 有界容量未生效：满时 push 应阻塞，骨架从不阻塞（capacity_ 只是存着）。
//   3. close() 是空操作：不置状态、不唤醒任何等待者；closed() 恒为 false。
//   4. push 在关闭后应抛 std::runtime_error，骨架不检查。
// 你的任务：按任务书的接口规格把这四个 TODO 补齐（需要 mutex + 两个条件变量）。
// ============================================================================
#include <cstddef>
#include <mutex>
#include <queue>

template <typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(std::size_t capacity = 0)
        : capacity_(capacity) {}

    void push(const T& v) {
        std::lock_guard<std::mutex> lk(m_);
        q_.push(v);                           // TODO 2/4：满阻塞与关闭检查缺失
    }

    bool pop(T& out) {
        std::lock_guard<std::mutex> lk(m_);
        if (q_.empty()) return false;         // TODO 1：这里应该睡等，而不是撒谎
        out = q_.front();
        q_.pop();
        return true;
    }

    void close() {
        // TODO 3：置关闭状态并唤醒全部等待者（生产者与消费者都要）。
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lk(m_);
        return q_.size();
    }

    bool closed() const { return false; }     // TODO 3：返回真实状态

private:
    std::queue<T> q_;
    std::size_t capacity_;                    // 0 = 无界
    mutable std::mutex m_;
};
