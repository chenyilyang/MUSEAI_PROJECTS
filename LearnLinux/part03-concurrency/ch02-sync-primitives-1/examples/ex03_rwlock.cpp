// ex03_rwlock.cpp —— 读写锁（pthread_rwlock）：读者共享、写者排他
// 场景：一份配置（version 与 value 必须恒满足 value = version * 10），
// 1 个写者反复整份更新，4 个读者高频读取——读者若用普通 mutex 只能串行读，
// 读写锁允许读者并行，且读者永不见"改了一半"的中间态。
// C++11 没有标准读写锁（std::shared_mutex 是 C++17），故用 pthread_rwlock；
// 线程本体仍用 std::thread——锁与线程 API 可以混搭。
// 编译基线：C++11
#include <atomic>
#include <cstdio>
#include <pthread.h>
#include <thread>
#include <vector>

struct Config {
    int version;
    int value;                 // 不变式：value == version * 10
};

static Config g_cfg = {0, 0};
static pthread_rwlock_t g_rw = PTHREAD_RWLOCK_INITIALIZER;
static std::atomic<bool> g_writerDone(false);
static std::atomic<int> g_violations(0);

static void writer() {
    for (int i = 0; i < 200; ++i) {
        pthread_rwlock_wrlock(&g_rw);          // 写锁：排他
        ++g_cfg.version;
        g_cfg.value = g_cfg.version * 10;      // 两步更新在锁内一气呵成
        pthread_rwlock_unlock(&g_rw);
    }
    g_writerDone = true;
}

static void reader() {
    long iter = 0;
    while (!g_writerDone.load()) {
        Config c;
        pthread_rwlock_rdlock(&g_rw);          // 读锁：与其它读者共享
        c = g_cfg;                             // 只在锁内做拷贝，检查放到锁外
        pthread_rwlock_unlock(&g_rw);
        if (c.value != c.version * 10) ++g_violations;   // 见中间态就记一次违规
        if ((++iter & 0x3FF) == 0) std::this_thread::yield();  // 别把写者饿死
    }
}

int main() {
    std::vector<std::thread> readers;
    for (int i = 0; i < 4; ++i) readers.push_back(std::thread(reader));
    std::thread w(writer);
    w.join();
    for (size_t i = 0; i < readers.size(); ++i) readers[i].join();

    std::printf("violations = %d\n", g_violations.load());
    pthread_rwlock_rdlock(&g_rw);
    std::printf("final: version = %d, value = %d\n", g_cfg.version, g_cfg.value);
    pthread_rwlock_unlock(&g_rw);
    return 0;
}
