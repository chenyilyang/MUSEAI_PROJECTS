// ex03_lifecycle.cpp —— 生命周期：joinable 状态、join 回收、detach 放养
// 编译基线：C++11
#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>

int main() {
    // —— join：joinable 在 join 前为真、之后为假；join 过的线程对象才可安全析构 ——
    std::thread t([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    });
    std::printf("before join: joinable = %d\n", t.joinable() ? 1 : 0);
    t.join();
    std::printf("after join: joinable = %d\n", t.joinable() ? 1 : 0);

    // —— detach：线程与对象脱钩，自己跑完自己收场 ——
    // 代价：你再也等不到它、拿不到它的结果，还必须保证它用到的东西活得比它久。
    // 这里用一个 atomic 计数让主线程"偷看"它是否干完（仅演示用，主线程靠它续命等待，
    // 真实代码里进程一旦退出，被 detach 的线程会被直接掐断在半路）。
    std::atomic<int> detachedCounter(0);
    std::thread d([&detachedCounter] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        detachedCounter = 1;
    });
    d.detach();
    std::printf("after detach: joinable = %d\n", d.joinable() ? 1 : 0);

    for (int i = 0; i < 200 && detachedCounter == 0; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::printf("detached counter = %d\n", detachedCounter.load());
    return 0;
}
