// 练习 4 参考答案：只用指针（不用下标）遍历数组求最大值
#include <cstdio>

int main() {
    int arr[5] = {5, 21, 9, 17, 13};
    int* end = arr + 5;

    int maxValue = *arr;               // 先假定首元素最大
    for (int* p = arr + 1; p != end; ++p) {
        if (*p > maxValue) {
            maxValue = *p;
        }
    }
    std::printf("max = %d\n", maxValue);   // 21
    return 0;
}
