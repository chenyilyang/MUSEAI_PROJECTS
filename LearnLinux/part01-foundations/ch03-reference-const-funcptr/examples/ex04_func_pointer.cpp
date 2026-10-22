// ex04_func_pointer.cpp —— 函数指针：把函数当数据传来传去
// 编译基线：C++11
#include <cstdio>

// 三个同签名的函数，供分发使用
int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int mul(int a, int b) { return a * b; }

// 给函数指针类型起个别名（using 是 C++11 的类型别名写法），签名可读得多
using BinaryOp = int (*)(int, int);

// 带回调的排序：升序还是降序由调用者传入的比较函数决定
void sortWith(int* arr, int n, bool (*less)(int, int)) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j + 1 < n - i; ++j) {
            if (less(arr[j + 1], arr[j])) {   // 回调：问调用者"该不该换"
                int t = arr[j]; arr[j] = arr[j + 1]; arr[j + 1] = t;
            }
        }
    }
}

bool ascending(int a, int b)  { return a < b; }    // a 更小就排前面
bool descending(int a, int b) { return a > b; }

void printArr(const int* arr, int n) {
    for (int i = 0; i < n; ++i) std::printf("%d ", arr[i]);
    std::printf("\n");
}

int main() {
    // 1) 函数指针数组做分发：按操作码选函数
    BinaryOp ops[3] = {add, sub, mul};
    const char* names[3] = {"add", "sub", "mul"};
    for (int i = 0; i < 3; ++i) {
        std::printf("6 %s 3 = %d\n", names[i], ops[i](6, 3));   // 9 / 3 / 18
    }

    // 2) 同一份排序代码，靠回调实现两种顺序
    int data1[5] = {4, 2, 9, 1, 7};
    sortWith(data1, 5, ascending);
    printArr(data1, 5);              // 1 2 4 7 9

    int data2[5] = {4, 2, 9, 1, 7};
    sortWith(data2, 5, descending);
    printArr(data2, 5);              // 9 7 4 2 1
    return 0;
}
