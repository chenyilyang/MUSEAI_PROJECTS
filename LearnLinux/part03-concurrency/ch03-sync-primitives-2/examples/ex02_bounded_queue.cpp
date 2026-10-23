// ex02_bounded_queue.cpp —— 有界队列：生产者-消费者的标准形态
// 容量有界带来"反压"：队列满时生产者也要睡。两个条件变量分工：
//   notEmpty_ 唤醒消费者，notFull_ 唤醒生产者。
// 关闭协议：close 后消费者取空存量才退出——阶段项目日志库的雏形。
// 编译基线：C++11
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class BoundedQueue {
public:
    explicit BoundedQueue(size_t cap) : cap_(cap), closed_(false), maxSeen_(0) {}

    void push(int v) {
        std::unique_lock<std::mutex> lk(m_);
        notFull_.wait(lk, [this] { return q_.size() < cap_; });   // 满则睡
        q_.push(v);
        if (q_.size() > maxSeen_) maxSeen_ = q_.size();
        lk.unlock();
        notEmpty_.notify_one();
    }

    // 取一个元素；返回 false 表示"已关闭且已取空"，消费者据此收尾
    bool pop(int& out) {
        std::unique_lock<std::mutex> lk(m_);
        notEmpty_.wait(lk, [this] { return !q_.empty() || closed_; });
        if (q_.empty()) return false;
        out = q_.front();
        q_.pop();
        lk.unlock();
        notFull_.notify_one();
        return true;
    }

    void close() {
        {
            std::lock_guard<std::mutex> lk(m_);
            closed_ = true;
        }
        notEmpty_.notify_all();   // 叫醒所有消费者来收尾（notify_one 会漏人）
        notFull_.notify_all();
    }

    size_t maxSeen() {
        std::lock_guard<std::mutex> lk(m_);
        return maxSeen_;
    }

private:
    size_t cap_;
    std::queue<int> q_;
    bool closed_;
    size_t maxSeen_;
    std::mutex m_;
    std::condition_variable notEmpty_;
    std::condition_variable notFull_;
};

static void producer(BoundedQueue& q, int lo, int hi) {   // 产出 [lo, hi]
    for (int v = lo; v <= hi; ++v) q.push(v);
}

static void consumer(BoundedQueue& q, long* sumOut, int* countOut) {
    long sum = 0;
    int cnt = 0;
    int v;
    while (q.pop(v)) {
        sum += v;
        ++cnt;
    }
    *sumOut = sum;
    *countOut = cnt;
}

int main() {
    BoundedQueue q(4);
    long sums[2] = {0, 0};
    int counts[2] = {0, 0};

    std::thread c0(consumer, std::ref(q), &sums[0], &counts[0]);
    std::thread c1(consumer, std::ref(q), &sums[1], &counts[1]);
    std::thread p0(producer, std::ref(q), 1, 500);        // 和 125250
    std::thread p1(producer, std::ref(q), 1001, 1500);    // 和 625250

    p0.join();
    p1.join();
    q.close();                                            // 生产完毕才关门
    c0.join();
    c1.join();

    int consumed = counts[0] + counts[1];
    long total = sums[0] + sums[1];
    std::printf("produced = 1000, consumed = %d\n", consumed);
    std::printf("consumed sum = %ld (expected 750500)\n", total);
    std::printf("max queue size = %zu (capacity 4)\n", q.maxSeen());
    return 0;
}
