// 练习 4 参考答案：StartGate——一次性发令枪（条件变量 + 谓词 + notify_all）
// 所有等待者等同一个事件：门开之前一个都过不去，门开之后全部放行，
// 且迟到的线程（门开后才来 wait）凭谓词直接通过、不再睡。
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

class StartGate {
public:
    StartGate() : open_(false) {}

    void wait() {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [this] { return open_; });   // 谓词：门已开则立即返回
    }

    void open() {
        {
            std::lock_guard<std::mutex> lk(m_);
            open_ = true;
        }
        cv_.notify_all();                          // 放行要叫醒所有人
    }

private:
    bool open_;
    std::mutex m_;
    std::condition_variable cv_;
};

int main() {
    StartGate gate;
    std::atomic<int> passed(0);
    std::vector<std::thread> pool;
    for (int i = 0; i < 4; ++i)
        pool.push_back(std::thread([&] {
            gate.wait();
            ++passed;
        }));

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::printf("before open: passed = %d\n", passed.load());   // 必须是 0
    int before = passed.load();

    gate.open();
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    std::printf("after open: passed = %d\n", passed.load());   // 必须是 4

    gate.wait();   // 门开之后再来的等待者：凭谓词直接通过（不挂住即证明）
    std::printf("late waiter passed immediately\n");
    return (before == 0 && passed.load() == 4) ? 0 : 1;
}
