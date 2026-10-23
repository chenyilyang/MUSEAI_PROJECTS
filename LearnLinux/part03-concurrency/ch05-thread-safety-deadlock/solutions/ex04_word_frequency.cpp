// 练习 4 参考答案：词频统计的线程安全版（对照练习题面里的无锁原版）
// 原版的病：std::map 的并发写既是数据竞争，还可能把红黑树结构写坏——
// 不是"数字不准"的问题，是可能直接崩溃/死循环的问题。
// 修法没有花活：一把 mutex 护住整个 map（临界区只圈 map 操作本身）。
// 自行验证：以 -fsanitize=thread 构建运行，应零报告；把锁去掉重跑，TSan 立刻报。
#include <cstdio>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

static std::map<std::string, int> g_counts;
static std::mutex g_m;

static void countWords() {
    static const char* kWords[4] = {"can", "hal", "someip", "vsomeip"};
    for (int r = 0; r < 250; ++r) {
        for (int w = 0; w < 4; ++w) {
            std::lock_guard<std::mutex> lk(g_m);
            ++g_counts[kWords[w]];
        }
    }
}

int main() {
    std::vector<std::thread> pool;
    for (int i = 0; i < 4; ++i) pool.push_back(std::thread(countWords));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();

    long total = 0;
    for (std::map<std::string, int>::const_iterator it = g_counts.begin(); it != g_counts.end(); ++it) {
        std::printf("%s: %d\n", it->first.c_str(), it->second);   // 每词 4×250 = 1000
        total += it->second;
    }
    std::printf("total = %ld\n", total);                          // 4000
    return total == 4000 ? 0 : 1;
}
