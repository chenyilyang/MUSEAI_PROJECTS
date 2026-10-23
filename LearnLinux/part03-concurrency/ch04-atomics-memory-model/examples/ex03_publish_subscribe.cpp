// ex03_publish_subscribe.cpp —— 发布-订阅：内存序的用武之地
// 模式：发布者先写一堆普通数据，最后置标志；订阅者盯着标志，一旦置起就读数据。
// 数据本身不是原子的，它的可见性靠标志的内存序"捎带"：
//   发布者 store 用 release —— 它之前的写，不许被重排到这次 store 之后；
//   订阅者 load  用 acquire —— 它之后的读，不许被重排到这次 load 之前。
// 两序相遇（synchronizes-with）：订阅者读到的必然是完整数据。
// 若换成 relaxed：x86 的强内存模型通常仍会"看起来正常"，
// 但 ARM 等弱序架构上可能先看见标志、后看见数据——车载 SoC 多为 ARM，这不是学究问题。
// 编译基线：C++11
#include <atomic>
#include <cstdio>
#include <thread>

static int g_data[4];                              // 普通数据，非原子
static std::atomic<bool> g_ready(false);           // 信使标志，原子

static void publisher() {
    for (int i = 0; i < 4; ++i) g_data[i] = (i + 1) * 111;   // 先写数据
    g_ready.store(true, std::memory_order_release);          // 再升旗（release）
}

static void subscriber() {
    int spins = 0;
    while (!g_ready.load(std::memory_order_acquire)) {       // 盯旗（acquire）
        if ((++spins & 0x3FF) == 0) std::this_thread::yield();
    }
    // 走到这里：发布者在 store 之前的一切写，对本线程必然可见
    std::printf("received: %d %d %d %d\n", g_data[0], g_data[1], g_data[2], g_data[3]);
    int sum = g_data[0] + g_data[1] + g_data[2] + g_data[3];
    std::printf("verified sum = %d\n", sum);                 // 111+222+333+444 = 1110
}

int main() {
    std::thread sub(subscriber);   // 先起订阅者，让它真的经历"等"
    std::thread pub(publisher);
    pub.join();
    sub.join();
    return 0;
}
