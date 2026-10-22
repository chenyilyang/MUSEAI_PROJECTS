// 练习 4 参考答案：在堆上克隆一个数组，改副本不影响原数组
#include <cstdio>

// 返回堆上的副本；调用者负责 delete[]
int* cloneArray(const int* src, int n) {
    int* copy = new int[n];
    for (int i = 0; i < n; ++i) {
        copy[i] = src[i];
    }
    return copy;
}

int main() {
    int src[3] = {1, 2, 3};
    int* copy = cloneArray(src, 3);

    copy[0] = 99;                      // 只改副本
    std::printf("src[0]  = %d\n", src[0]);    // 1，原数组不受影响
    std::printf("copy[0] = %d\n", copy[0]);   // 99

    delete[] copy;
    copy = nullptr;
    return 0;
}
