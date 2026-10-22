// 练习 3 参考答案：用二级指针把"分配好的数组"交回给调用者
#include <cstdio>

// 成功返回 true；out 接收新数组的地址，元素填 1..n
bool allocArray(int n, int** out) {
    if (n <= 0 || out == nullptr) {
        return false;
    }
    *out = new int[n];
    for (int i = 0; i < n; ++i) {
        (*out)[i] = i + 1;
    }
    return true;
}

int main() {
    int* arr = nullptr;
    if (allocArray(5, &arr)) {
        for (int i = 0; i < 5; ++i) {
            std::printf("%d ", arr[i]);
        }
        std::printf("\n");             // 1 2 3 4 5
        delete[] arr;
        arr = nullptr;
    }
    return 0;
}
