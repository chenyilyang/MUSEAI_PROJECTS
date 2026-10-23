// 练习 4 参考答案：pthread 版分段求和，结果经 void* 返回
#include <cstdio>
#include <pthread.h>

struct Range { long lo; long hi; };       // [lo, hi)

static void* partialSum(void* arg) {
    Range* r = (Range*)arg;
    long s = 0;
    for (long i = r->lo; i < r->hi; ++i) s += i;
    long* out = new long(s);              // 结果上堆，join 方回收
    return out;
}

int main() {
    const long kTotal = 1000000;
    const int kThreads = 4;
    pthread_t th[kThreads];
    Range ranges[kThreads];
    long chunk = (kTotal + 1) / kThreads;
    for (int i = 0; i < kThreads; ++i) {
        ranges[i].lo = 1 + i * chunk;
        ranges[i].hi = ranges[i].lo + chunk;
        if (ranges[i].hi > kTotal + 1) ranges[i].hi = kTotal + 1;
        if (pthread_create(&th[i], NULL, partialSum, &ranges[i]) != 0) {
            std::perror("pthread_create");
            return 1;
        }
    }
    long total = 0;
    for (int i = 0; i < kThreads; ++i) {
        void* ret = NULL;
        pthread_join(th[i], &ret);
        total += *(long*)ret;
        delete (long*)ret;
    }
    std::printf("pthread total = %ld\n", total);           // 500000500000
    return 0;
}
