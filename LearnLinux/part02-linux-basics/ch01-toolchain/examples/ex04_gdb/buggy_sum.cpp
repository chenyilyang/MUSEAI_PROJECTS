// buggy_sum.cpp —— ex04 的调试对象：求 1..10 之和。
// 结果会是 56 而不是 55 —— 用 GDB 按正文会话找病根。
#include <cstdio>

int main() {
    int sum = 1;                       // ← 病根藏在这里（初值本应为 0）
    for (int i = 1; i <= 10; ++i) {
        sum += i;
    }
    std::printf("sum 1..10 = %d\n", sum);
    return 0;
}
