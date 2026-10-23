// ex02_args_and_ref.cpp —— 线程参数传递三式：值、std::ref、指针
// 先记住本质：std::thread 会把参数"衰变拷贝"进线程内部存储，
// 所以默认永远是传值；想传引用必须用 std::ref 明确表态（或传指针）。
// 编译基线：C++11
#include <cstdio>
#include <functional>   // std::ref
#include <thread>

static void setByValue(int v) { v = 99; }        // 改的是副本
static void setByRef(int& r) { r = 99; }         // 需要调用方用 std::ref 送进来
static void setByPointer(int* p) { *p = 7; }     // 指针本身是拷贝，指向物是共享的

int main() {
    int x = 1;
    std::thread t1(setByValue, x);
    t1.join();
    std::printf("after value-arg thread: x = %d\n", x);        // 仍是 1

    int y = 2;
    std::thread t2(setByRef, std::ref(y));                     // 不用 std::ref 则编译不过
    t2.join();
    std::printf("after ref-arg thread: y = %d\n", y);          // 变为 99

    int z = 3;
    std::thread t3(setByPointer, &z);
    t3.join();
    std::printf("after pointer-arg thread: z = %d\n", z);      // 变为 7
    return 0;
}
