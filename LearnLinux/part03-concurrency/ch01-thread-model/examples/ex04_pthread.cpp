// ex04_pthread.cpp —— POSIX 线程对照：pthread_create / pthread_join
// 与 std::thread 的最大观感差别：返回值走 void*（这里返回堆上 int），
// 一切都要手动：创建、等待、回收返回值内存。std::thread 底层在 Linux 上正是 pthread。
// 编译基线：C++11
#include <cstdio>
#include <pthread.h>

static void* worker(void* arg) {
    long base = (long)arg;                 // 参数同样是"一个指针塞一切"
    int* result = new int((int)base + 2);   // 结果放堆上，交给 join 的人回收
    return result;                          // 等价于 pthread_exit(result)
}

int main() {
    pthread_t th;
    if (pthread_create(&th, NULL, worker, (void*)40L) != 0) {
        std::perror("pthread_create");
        return 1;
    }
    void* ret = NULL;
    pthread_join(th, &ret);                 // join 顺带取回返回值
    std::printf("pthread result = %d\n", *(int*)ret);   // 42
    delete (int*)ret;
    return 0;
}
