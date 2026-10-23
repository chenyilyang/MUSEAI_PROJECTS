// ex01_thread_basics.cpp —— std::thread 入门：创建、id、join、分槽汇总
// 纪律先行：每个 worker 只写属于自己的槽位（不共享可变状态），
// 主线程 join 之后再统一读——本章的"安全姿势"，竞争版在 ex05。
// 编译基线：C++11（链接需 -pthread，CMake 的 Threads::Threads 已处理）
#include <cstdio>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

struct Slot {
    long sum;
    std::string id;
};

static void ladderSum(int idx, int n, Slot* slot) {
    long s = 0;
    for (int i = 1; i <= n; ++i) s += i;
    slot->sum = s;
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    slot->id = oss.str();          // id 也先存槽里，打印统一由主线程做——输出不交错
}

int main() {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::printf("main thread id = %s\n", oss.str().c_str());

    const int kWorkers = 4;
    std::vector<Slot> slots((size_t)kWorkers);
    std::vector<std::thread> pool;
    for (int i = 0; i < kWorkers; ++i) {
        // 第 i 个 worker 算 1..(i+1)*10 的和
        pool.push_back(std::thread(ladderSum, i, (i + 1) * 10, &slots[(size_t)i]));
    }
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();   // 逐个回收

    for (int i = 0; i < kWorkers; ++i) {
        std::printf("worker %d thread id = %s\n", i, slots[(size_t)i].id.c_str());
        std::printf("slot %d = %ld\n", i, slots[(size_t)i].sum);
    }
    return 0;
}
