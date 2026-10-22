// 练习 3 参考答案：第 3 章的 clampValue 升级为函数模板
#include <cstdio>

template <typename T>
T clampT(T v, T lo, T hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

int main() {
    std::printf("int:    %d\n", clampT(150, 0, 100));        // 100
    std::printf("double: %f\n", clampT(2.7, 0.0, 2.5));      // 2.5
    std::printf("int:    %d\n", clampT(42, 0, 100));         // 42
    return 0;
}
