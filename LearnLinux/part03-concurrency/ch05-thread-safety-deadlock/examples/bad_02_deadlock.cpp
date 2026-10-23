// bad_02_deadlock.cpp —— 反例：反序加锁的教科书死锁
// 线程 1 先 A 后 B，线程 2 先 B 后 A；中间的 sleep 保证双方都先攥住第一把锁，
// 再去够对方手里的那把——互斥、持有并等待、不可剥夺、循环等待，四条件凑齐。
// 运行预期：两行 "holding..., waiting..." 打印后永久挂住（timeout 收尸）。
// 现场诊断（正文演示）：gdb -p <pid> 后 thread apply all bt，
// 两个线程的栈会停在各自的 lock 调用上，环一目了然。
// 编译基线：C++11
#include <chrono>
#include <cstdio>
#include <mutex>
#include <thread>

static std::mutex g_a;
static std::mutex g_b;

static void thread1() {
    g_a.lock();
    std::printf("thread 1: holding A, waiting for B\n");
    std::fflush(stdout);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 保证对方也已就位
    g_b.lock();                                   // 永远等不到
    g_b.unlock();
    g_a.unlock();
}

static void thread2() {
    g_b.lock();
    std::printf("thread 2: holding B, waiting for A\n");
    std::fflush(stdout);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    g_a.lock();                                   // 永远等不到
    g_a.unlock();
    g_b.unlock();
}

int main() {
    std::thread t1(thread1);
    std::thread t2(thread2);
    t1.join();                                    // 主线程陪挂
    t2.join();
    std::printf("never reached\n");
    return 0;
}
