// 练习 3 参考答案：用引用把数值钳制在 [lo, hi] 区间内
#include <cstdio>

void clampValue(int& v, int lo, int hi) {
    if (v < lo) v = lo;
    if (v > hi) v = hi;
}

int main() {
    int a = 150, b = -5, c = 42;
    clampValue(a, 0, 100);
    clampValue(b, 0, 100);
    clampValue(c, 0, 100);
    std::printf("a=%d, b=%d, c=%d\n", a, b, c);   // 100, 0, 42
    return 0;
}
