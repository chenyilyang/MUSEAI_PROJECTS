// queue_check —— BlockingQueue<T> 验收程序（starter 与 solutions 共用同一份）
// 场景一（无界）：单生产者入队 1..100 后 close，消费者必须取全、且最终收到结束信号。
// 场景二（有界，容量 2）：装满后第三次 push 必须真阻塞，直到消费者取走一个才完成；
//                         元素顺序保持 FIFO。
#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>

#include "blocking_queue.h"

int main() {
    bool allOk = true;

    // —— 场景一：无界 + close 排空 ——
    {
        BlockingQueue<int> q;
        long sum = 0;
        int cnt = 0;
        bool sawEnd = false;
        std::thread consumer([&] {
            int v;
            while (q.pop(v)) {
                sum += v;
                ++cnt;
            }
            sawEnd = true;                    // pop 返回 false 才出循环
        });
        for (int i = 1; i <= 100; ++i) q.push(i);
        q.close();
        consumer.join();
        std::printf("unbounded: count = %d, sum = %ld\n", cnt, sum);
        if (cnt == 100 && sum == 5050 && sawEnd)
            std::printf("unbounded: pop returns false after drain\n");
        else
            allOk = false;
    }

    // —— 场景二：有界容量 2，满时阻塞 ——
    {
        BlockingQueue<int> q(2);
        q.push(1);
        q.push(2);
        std::atomic<bool> thirdDone(false);
        std::thread pusher([&] {
            q.push(3);                        // 队列已满：必须阻塞在这里
            thirdDone.store(true);
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        bool blocked = !thirdDone.load();
        int v1 = 0, v2 = 0, v3 = 0;
        q.pop(v1);                            // 取走一个，第三次 push 才能完成
        pusher.join();
        q.pop(v2);
        q.pop(v3);
        if (blocked)
            std::printf("bounded: third push blocked while full\n");
        else
            allOk = false;
        std::printf("bounded: order preserved %d %d %d\n", v1, v2, v3);
        if (!(v1 == 1 && v2 == 2 && v3 == 3)) allOk = false;
    }

    return allOk ? 0 : 1;
}
