// ex01：线程池（C++11 基线）——工作队列 + 条件变量 + 两种关闭语义
//   submit      把可调用对象包成 packaged_task 入队，返回 future 取结果
//   shutdown    排空退出：不再接新任务，队列里的任务全部做完才停（析构默认走这条）
//   shutdownNow 立即退出：队列里没开工的任务直接丢弃——它们的 packaged_task 被销毁，
//               对应 future 以 broken_promise 异常收场（在下面的演示里数得清清楚楚）
// 结构与第 3 章 BlockingQueue 同源：mutex + condition_variable + 谓词等待，
// 这里把队列元素换成"无参任务"，返回值通道交给 packaged_task/future。
//
// 一个真实的接口坑：packaged_task 只可移动、不可拷贝，而 std::function 要求
// 目标可拷贝，所以任务入队要经 shared_ptr 中转（见 submit），这是所有
// C++11 线程池实现都要过的同一道坎。
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(std::size_t workers) : stopping_(false) {
        for (std::size_t i = 0; i < workers; ++i)
            threads_.push_back(std::thread(&ThreadPool::workerLoop, this));
    }

    ~ThreadPool() { shutdown(); }            // 兜底：忘了关，析构按排空语义关

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // 投递一个任务，拿回 future；任务的返回值（或它抛出的异常）都从 future 里取。
    template <typename F>
    std::future<typename std::result_of<F()>::type> submit(F&& f) {
        typedef typename std::result_of<F()>::type R;
        std::shared_ptr<std::packaged_task<R()> > task(
            new std::packaged_task<R()>(std::forward<F>(f)));
        std::future<R> fut = task->get_future();
        {
            std::lock_guard<std::mutex> lk(m_);
            if (stopping_) throw std::runtime_error("submit on stopped pool");
            tasks_.push([task] { (*task)(); });
        }
        cv_.notify_one();
        return fut;
    }

    // 排空退出：置位停止标志，工作线程把队列做空后自行退出，再逐个 join。
    void shutdown() {
        {
            std::lock_guard<std::mutex> lk(m_);
            if (stopping_) return;            // 两种关闭都只认第一次调用
            stopping_ = true;
        }
        cv_.notify_all();
        joinAll();
    }

    // 立即退出：置位后先把队列清空（未开工任务在此被销毁），再叫醒并 join。
    // 正在执行的任务不会被打断——线程没有强杀（见练习 4），只能等它自己跑完。
    void shutdownNow() {
        {
            std::lock_guard<std::mutex> lk(m_);
            if (stopping_) return;
            stopping_ = true;
            std::queue<std::function<void()> > empty;
            tasks_.swap(empty);               // empty 出作用域时销毁全部未开工任务
        }
        cv_.notify_all();
        joinAll();
    }

private:
    void workerLoop() {
        for (;;) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lk(m_);
                cv_.wait(lk, [this] { return stopping_ || !tasks_.empty(); });
                if (tasks_.empty()) return;   // 已停止且排空（或被立即退出清空）
                task = tasks_.front();
                tasks_.pop();
            }
            task();
        }
    }

    void joinAll() {
        for (std::size_t i = 0; i < threads_.size(); ++i)
            if (threads_[i].joinable()) threads_[i].join();
    }

    std::vector<std::thread> threads_;
    std::queue<std::function<void()> > tasks_;
    bool stopping_;
    std::mutex m_;
    std::condition_variable cv_;
};

int main() {
    // —— 主演示：4 个工作线程，20 个平方任务，future 按投递顺序取回 ——
    ThreadPool pool(4);
    std::vector<std::future<int> > futs;
    for (int i = 0; i < 20; ++i)
        futs.push_back(pool.submit([i] { return i * i; }));

    std::printf("results:");
    long sum = 0;
    for (int i = 0; i < 20; ++i) {
        int v = futs[i].get();                // 按投递顺序取，输出与调度无关、恒定
        std::printf(" %d", v);
        sum += v;
    }
    std::printf("\nsum of squares 0..19 = %ld (expected 2470)\n", sum);
    pool.shutdown();                          // 排空退出；此处队列早已空

    // —— 立即退出演示：1 个工作线程被长任务占住，5 个任务堵在队列里 ——
    ThreadPool pool2(1);
    std::atomic<bool> started(false);
    std::atomic<bool> release(false);
    std::atomic<int> ran(0);
    std::future<void> first = pool2.submit([&] {
        started.store(true);
        while (!release.load())
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        ran.fetch_add(1);
    });
    std::vector<std::future<void> > queued;
    for (int i = 0; i < 5; ++i)
        queued.push_back(pool2.submit([&] { ran.fetch_add(1); }));

    while (!started.load())                   // 等长任务确实开工，演示前提才成立
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // shutdownNow 会在 join 处等长任务结束，所以另开一条线程调用它；
    // 它先清队列（5 个任务被销毁），主线程随后放行长任务，join 随之返回。
    std::thread closer([&] { pool2.shutdownNow(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    release.store(true);
    closer.join();
    first.get();                              // 长任务正常完成

    int broken = 0;
    for (std::size_t i = 0; i < queued.size(); ++i) {
        try {
            queued[i].get();
        } catch (const std::future_error& e) {
            if (e.code() == std::future_errc::broken_promise) ++broken;
        }
    }
    std::printf("shutdownNow demo: ran = %d, broken futures = %d (expected 1 and 5)\n",
                ran.load(), broken);
    return (sum == 2470 && ran.load() == 1 && broken == 5) ? 0 : 1;
}
