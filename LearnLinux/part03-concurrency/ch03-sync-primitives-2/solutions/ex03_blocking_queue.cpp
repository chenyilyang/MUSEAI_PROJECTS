// 练习 3 参考答案：BlockingQueue<T>——无界阻塞队列类模板
// 阶段项目（任务队列 + 日志库）的地基接口在此定型：
//   push    生产者放入，永不阻塞（无界）
//   pop     消费者取出；队列空则睡等；已关闭且取空时返回 false
//   close   关门：唤醒所有等待者，存量仍可取完
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

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

int main() {
    // —— 用 int 验证并发总量守恒 ——
    BlockingQueue<int> q;
    long sum = 0;
    int cnt = 0;
    std::thread consumer([&] {
        int v;
        while (q.pop(v)) {
            sum += v;
            ++cnt;
        }
    });
    std::thread producer([&] {
        for (int i = 1; i <= 100; ++i) q.push(i);
        q.close();
    });
    producer.join();
    consumer.join();
    std::printf("int queue: count = %d, sum = %ld\n", cnt, sum);   // 100, 5050

    // —— 用 std::string 验证模板对任意可拷贝类型的通用性 ——
    BlockingQueue<std::string> sq;
    sq.push("hello");
    sq.push(" ");
    sq.push("world");
    sq.close();
    std::string joined, w;
    while (sq.pop(w)) joined += w;
    std::printf("string queue: %s\n", joined.c_str());             // hello world

    return (cnt == 100 && sum == 5050 && joined == "hello world") ? 0 : 1;
}
