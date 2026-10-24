#pragma once
// ============================================================================
// ThreadPool —— 第 6 章 ex01 的定型版（毕业项目直接复用，签名不要改）
// 建立在本项目的 BlockingQueue 之上（练习 2 的结论在这里成为正式结构）：
//   explicit ThreadPool(std::size_t workers);
//   submit(F) -> std::future<...>   投递任务；关闭后投递抛 std::runtime_error
//   shutdown()                      排空退出：队列任务全做完才返回
//   shutdownNow()                   立即退出：排队中的任务被抛弃（其 future 以
//                                   broken_promise 收场），正在执行的跑完
// 析构 = shutdown() 兜底。
// ============================================================================
#include <atomic>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

#include "blocking_queue.h"

class ThreadPool {
public:
    explicit ThreadPool(std::size_t workers)
        : stopped_(false), discard_(false) {
        for (std::size_t i = 0; i < workers; ++i)
            threads_.push_back(std::thread(&ThreadPool::workerLoop, this));
    }

    ~ThreadPool() { shutdown(); }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template <typename F>
    std::future<typename std::result_of<F()>::type> submit(F&& f) {
        typedef typename std::result_of<F()>::type R;
        std::shared_ptr<std::packaged_task<R()> > task(
            new std::packaged_task<R()>(std::forward<F>(f)));
        std::future<R> fut = task->get_future();
        if (stopped_.load()) throw std::runtime_error("submit on stopped pool");
        try {
            queue_.push([task] { (*task)(); });
        } catch (const std::runtime_error&) {
            throw std::runtime_error("submit on stopped pool");
        }
        return fut;
    }

    // 排空退出：关队列的门，工作线程取完存量（pop 返回 false）后离场。
    void shutdown() {
        bool expected = false;
        if (!stopped_.compare_exchange_strong(expected, true)) return;
        queue_.close();
        joinAll();
    }

    // 立即退出：先立抛弃标志再关门——工作线程把排队任务逐个取出并丢弃
    // （packaged_task 随之销毁，future 收到 broken_promise），手头任务跑完。
    void shutdownNow() {
        bool expected = false;
        if (!stopped_.compare_exchange_strong(expected, true)) return;
        discard_.store(true);
        queue_.close();
        joinAll();
    }

private:
    void workerLoop() {
        std::function<void()> task;
        while (queue_.pop(task)) {
            if (!discard_.load()) task();
        }
    }

    void joinAll() {
        for (std::size_t i = 0; i < threads_.size(); ++i)
            if (threads_[i].joinable()) threads_[i].join();
    }

    BlockingQueue<std::function<void()> > queue_;
    std::vector<std::thread> threads_;
    std::atomic<bool> stopped_;
    std::atomic<bool> discard_;
};
