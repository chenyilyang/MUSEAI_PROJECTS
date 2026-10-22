// 练习 4 参考答案：buggy_avg 的修正版
// 病根：sum 与 count 都是 int，相除是整数除法，小数部分被截掉。
// 改法：相除前把 sum 转成 double。
#include <cstdio>

int main() {
    int values[] = {1, 2};
    int n = 2;
    int sum = 0;
    for (int i = 0; i < n; ++i) sum += values[i];
    double avg = static_cast<double>(sum) / n;   // 先转 double 再除
    std::printf("avg = %f\n", avg);              // 1.500000
    return 0;
}
