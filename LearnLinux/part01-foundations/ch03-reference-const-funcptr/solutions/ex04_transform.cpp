// 练习 4 参考答案：用函数指针回调批量变换数组元素
#include <cstdio>

void transform(int* arr, int n, int (*fn)(int)) {
    for (int i = 0; i < n; ++i) {
        arr[i] = fn(arr[i]);           // 对每个元素调用回调
    }
}

int square(int x) { return x * x; }
int negate(int x) { return -x; }

void printArr(const int* arr, int n) {
    for (int i = 0; i < n; ++i) std::printf("%d ", arr[i]);
    std::printf("\n");
}

int main() {
    int a[4] = {1, 2, 3, 4};
    transform(a, 4, square);
    printArr(a, 4);                    // 1 4 9 16

    int b[4] = {1, 2, 3, 4};
    transform(b, 4, negate);
    printArr(b, 4);                    // -1 -2 -3 -4
    return 0;
}
