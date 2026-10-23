// 练习 3 参考答案：线程安全计数器类
// 纪律：数据与锁同居一室、对外只暴露方法——调用方想绕过锁直接摸数据都做不到。
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

class ThreadSafeCounter {
public:
    ThreadSafeCounter() : value_(0) {}
    void inc() {
        std::lock_guard<std::mutex> lk(m_);
        ++value_;
    }
    void add(long n) {
        std::lock_guard<std::mutex> lk(m_);
        value_ += n;
    }
    long value() const {                       // 读也要上锁：读的是可变共享状态
        std::lock_guard<std::mutex> lk(m_);
        return value_;
    }
private:
    mutable std::mutex m_;                     // mutable：const 方法里也要能锁
    long value_;
};

int main() {
    ThreadSafeCounter counter;
    std::vector<std::thread> pool;
    for (int i = 0; i < 4; ++i)
        pool.push_back(std::thread([&counter] {
            for (int j = 0; j < 250000; ++j) counter.inc();
        }));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    std::printf("counter = %ld\n", counter.value());       // 1000000
    return counter.value() == 1000000 ? 0 : 1;
}
