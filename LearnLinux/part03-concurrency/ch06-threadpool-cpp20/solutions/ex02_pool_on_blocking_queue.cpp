// 练习 2 参考答案：把第 3 章的 BlockingQueue<T> 改造成线程池的工作队列
// 对照 ex01 的手写队列版，两处变化：
//   1. 工作线程的取任务动作就是 q_.pop(task)——pop 返回 false 即"已关闭且取空"，
//      天然就是退出信号，不再需要单独的谓词与停止标志来表达"排空"；
//   2. 关闭协议直接复用队列的 close()：关门后存量任务仍被取完（排空退出语义），
//      取空后 pop 返回 false，工作线程干净离场。少一份状态，就少一类不一致。
#include <atomic>
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

// —— 第 3 章练习 3 定型的 BlockingQueue<T>，接口原样（push / pop / close） ——
template <typename T>
class BlockingQueue {
public:
    BlockingQueue() : closed_(false) {}

    void push(const T& v) {
        {
            std::lock_guard<std::mutex> lk(m_);
            q_.push(v);
        }
        notEmpty_.notify_one();
    }

    bool pop(T& out) {
        std::unique_lock<std::mutex> lk(m_);
        notEmpty_.wait(lk, [this] { return !q_.empty() || closed_; });
        if (q_.empty()) return false;         // 已关闭且取空
        out = q_.front();
        q_.pop();
        return true;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lk(m_);
            closed_ = true;
        }
        notEmpty_.notify_all();               // 收尾必须叫醒所有人
    }

private:
    std::queue<T> q_;
    bool closed_;
    std::mutex m_;
    std::condition_variable notEmpty_;
};

class BqThreadPool {
public:
    explicit BqThreadPool(std::size_t workers) : stopped_(false) {
        for (std::size_t i = 0; i < workers; ++i)
            threads_.push_back(std::thread(&BqThreadPool::workerLoop, this));
    }

    ~BqThreadPool() { shutdown(); }

    BqThreadPool(const BqThreadPool&) = delete;
    BqThreadPool& operator=(const BqThreadPool&) = delete;

    template <typename F>
    std::future<typename std::result_of<F()>::type> submit(F&& f) {
        typedef typename std::result_of<F()>::type R;
        std::shared_ptr<std::packaged_task<R()> > task(
            new std::packaged_task<R()>(std::forward<F>(f)));
        std::future<R> fut = task->get_future();
        if (stopped_.load()) throw std::runtime_error("submit on stopped pool");
        queue_.push([task] { (*task)(); });
        return fut;
    }

    // 排空退出：关队列的门——存量任务被取完后，pop 返回 false，线程离场。
    void shutdown() {
        bool expected = false;
        if (!stopped_.compare_exchange_strong(expected, true)) return;
        queue_.close();
        for (std::size_t i = 0; i < threads_.size(); ++i)
            if (threads_[i].joinable()) threads_[i].join();
    }

private:
    void workerLoop() {
        std::function<void()> task;
        while (queue_.pop(task)) task();      // 取空且已关闭时 pop 返回 false
    }

    BlockingQueue<std::function<void()> > queue_;
    std::vector<std::thread> threads_;
    std::atomic<bool> stopped_;
};

int main() {
    BqThreadPool pool(4);
    std::vector<std::future<int> > futs;
    for (int i = 0; i < 20; ++i)
        futs.push_back(pool.submit([i] { return i * i; }));
    long sum = 0;
    for (int i = 0; i < 20; ++i) sum += futs[i].get();
    pool.shutdown();
    std::printf("bq pool: sum = %ld (expected 2470)\n", sum);
    return sum == 2470 ? 0 : 1;
}
