#pragma once
// ============================================================================
// 【骨架】ThreadPool —— 接口与第 6 章定型的池一致，但骨架建立在骨架版
// BlockingQueue 之上：那里的 pop 不阻塞（空了就返回 false），于是本池的
// 工作线程一启动就会因为"队列暂时没货"而集体离场，任务留在队列里无人执行，
// 对应的 future 在池销毁时以 broken_promise 收场。这不是池本身的逻辑错，
// 是它脚下队列的关闭/等待协议没做完——先修 blocking_queue.h，这个池不用改
// 就能跟着变对（这正是把队列定型成基础设施的意义）。
// ============================================================================
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
    explicit ThreadPool(std::size_t workers) : stopping_(false) {
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
        if (stopping_) throw std::runtime_error("submit on stopped pool");
        queue_.push([task] { (*task)(); });
        return fut;
    }

    void shutdown() {
        if (stopping_) return;
        stopping_ = true;
        queue_.close();
        joinAll();
    }

    void shutdownNow() { shutdown(); }        // 骨架未区分两种语义（见任务书）

private:
    void workerLoop() {
        std::function<void()> task;
        while (queue_.pop(task)) task();      // pop 的语义对了，这个循环就是对的
    }

    void joinAll() {
        for (std::size_t i = 0; i < threads_.size(); ++i)
            if (threads_[i].joinable()) threads_[i].join();
    }

    BlockingQueue<std::function<void()> > queue_;
    std::vector<std::thread> threads_;
    bool stopping_;
};
