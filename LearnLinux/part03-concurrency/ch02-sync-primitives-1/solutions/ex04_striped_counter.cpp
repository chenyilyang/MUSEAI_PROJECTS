// 练习 4 参考答案：分片计数器——把一把大锁的争抢摊薄到 8 个分片
// 每个线程按自己的 thread id 哈希落到某个分片，只锁那一片；
// 求和时按固定顺序锁住全部片（顺序一致是防死锁的预习，第 5 章展开）。
#include <cstdio>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

class StripedCounter {
public:
    void inc() {
        size_t h = std::hash<std::thread::id>()(std::this_thread::get_id());
        Shard& s = shards_[h % kShards];
        std::lock_guard<std::mutex> lk(s.m);
        ++s.v;
    }
    long value() const {
        std::vector<std::unique_lock<std::mutex> > locks;
        for (int i = 0; i < kShards; ++i)
            locks.push_back(std::unique_lock<std::mutex>(shards_[i].m));  // 按序全锁
        long total = 0;
        for (int i = 0; i < kShards; ++i) total += shards_[i].v;
        return total;
    }
private:
    static const int kShards = 8;
    struct Shard {
        Shard() : v(0) {}
        mutable std::mutex m;
        long v;
    };
    Shard shards_[kShards];
};

int main() {
    StripedCounter counter;
    std::vector<std::thread> pool;
    for (int i = 0; i < 4; ++i)
        pool.push_back(std::thread([&counter] {
            for (int j = 0; j < 250000; ++j) counter.inc();
        }));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    std::printf("striped total = %ld\n", counter.value());   // 1000000
    return counter.value() == 1000000 ? 0 : 1;
}
