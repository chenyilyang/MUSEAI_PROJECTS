// bad_01_forgot_join.cpp —— 反例：joinable 的 std::thread 对象被析构
// std::thread 的纪律：对象析构时若仍 joinable（没 join 也没 detach），
// 直接 std::terminate —— 不是警告，是当场 abort。
// 运行预期：打印第一行后 abort（退出码 134），stderr 出现 "terminate called"。
#include <chrono>
#include <cstdio>
#include <thread>

int main() {
    {
        std::thread t([] {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        });
        std::printf("leaving scope without join/detach...\n");
        std::fflush(stdout);
        // 出作用域：t 被析构且仍 joinable → std::terminate
    }
    std::printf("never reached\n");
    return 0;
}
