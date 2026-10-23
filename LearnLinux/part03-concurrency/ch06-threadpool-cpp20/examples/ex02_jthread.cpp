// ex02：C++20 专题（一）—— jthread 与 stop_token 协作取消【-std=c++20 构建】
// 与 std::thread 的三点差别，全在这一小段里：
//   1. 析构自动 request_stop + join——作用域结束即收尾，没有"忘 join 就 terminate"
//      （第 1 章 bad_01 的坑），也不必手工判 joinable；
//   2. 任务签名带 std::stop_token 首参时，jthread 自动把自己的令牌传进来，
//      任务在循环里轮询 stop_requested() 协作退出；
//   3. 取消是"请求"不是"强杀"：任务不看令牌，谁来也停不下来（练习 4）。
#include <chrono>
#include <cstdio>
#include <stop_token>
#include <thread>

static void worker(std::stop_token st) {
    long ticks = 0;
    while (!st.stop_requested()) {
        ++ticks;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::printf("worker: stop requested, exiting at tick %ld\n", ticks);
}

int main() {
    {
        std::jthread t(worker);              // stop_token 被自动注入 worker
        std::this_thread::sleep_for(std::chrono::milliseconds(320));
        t.request_stop();                    // 只是"举手申请"，worker 看到才退
    }                                        // 析构：自动 request_stop + join
    std::printf("main: jthread auto-joined on scope exit\n");
    return 0;
}
