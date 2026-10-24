#pragma once
// ============================================================================
// BlockingQueue<T> —— 第三部分定型接口（毕业项目直接复用，签名不要改）
//
//   explicit BlockingQueue(std::size_t capacity = 0);   // 0 = 无界
//   void push(const T& v);   // 有界且满时阻塞；队列已关闭时抛 std::runtime_error
//   bool pop(T& out);        // 阻塞等待；返回 false 仅当"已关闭且已取空"
//   void close();            // 关门：唤醒全部等待者；存量仍可取完（排空语义）
//   std::size_t size() const;
//   bool closed() const;
//
// 设计要点（第 3 章关闭协议的定型版）：
//   - 两个条件变量分工：notEmpty_ 叫醒消费者，notFull_ 叫醒被"满"挡住的生产者；
//   - close() 必须 notify_all 两边——被"满"挡住的生产者醒来后发现已关闭，
//     以异常收场，而不是永远睡下去；
//   - 谓词永远写全（!empty || closed / !full || closed），虚假唤醒自动免疫。
// ============================================================================
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <queue>
#include <stdexcept>

template <typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(std::size_t capacity = 0)
        : capacity_(capacity), closed_(false) {}

    BlockingQueue(const BlockingQueue&) = delete;
    BlockingQueue& operator=(const BlockingQueue&) = delete;

    void push(const T& v) {
        {
            std::unique_lock<std::mutex> lk(m_);
            notFull_.wait(lk, [this] { return closed_ || !fullLocked(); });
            if (closed_) throw std::runtime_error("push on closed BlockingQueue");
            q_.push(v);
        }
        notEmpty_.notify_one();
    }

    bool pop(T& out) {
        {
            std::unique_lock<std::mutex> lk(m_);
            notEmpty_.wait(lk, [this] { return closed_ || !q_.empty(); });
            if (q_.empty()) return false;    // 已关闭且取空：结束信号
            out = q_.front();
            q_.pop();
        }
        notFull_.notify_one();
        return true;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lk(m_);
            closed_ = true;
        }
        notEmpty_.notify_all();
        notFull_.notify_all();
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lk(m_);
        return q_.size();
    }

    bool closed() const {
        std::lock_guard<std::mutex> lk(m_);
        return closed_;
    }

private:
    bool fullLocked() const { return capacity_ != 0 && q_.size() >= capacity_; }

    std::queue<T> q_;
    std::size_t capacity_;                    // 0 = 无界
    bool closed_;
    mutable std::mutex m_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
};
