// fixed_sum.cpp —— ex04 修正版：初值改回 0，输出 55
#include <cstdio>

int main() {
    int sum = 0;
    for (int i = 1; i <= 10; ++i) {
        sum += i;
    }
    std::printf("sum 1..10 = %d\n", sum);
    return 0;
}
