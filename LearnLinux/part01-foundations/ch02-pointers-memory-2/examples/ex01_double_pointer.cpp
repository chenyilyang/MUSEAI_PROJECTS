// ex01_double_pointer.cpp —— 多级指针：让函数"改指向"
// 编译基线：C++11
#include <cstdio>

// 场景：函数负责在堆上建好一个数组，再把数组交给调用者。
// 函数要修改的是调用者的"指针本身"，所以参数必须是指针的指针 int**。
void buildScores(int** out, int n) {
    *out = new int[n];                 // *out 就是调用者的指针，给它赋新地址
    for (int i = 0; i < n; ++i) {
        (*out)[i] = (i + 1) * 10;      // 注意括号：先解引用拿到数组，再下标
    }
}

int main() {
    // 先热个身：二级指针就是"指向指针的指针"
    int x = 1;
    int* p = &x;
    int** pp = &p;
    **pp = 2;                          // 顺着两级地址找到 x
    std::printf("x = %d\n", x);        // 2

    // 主场景：scores 本来什么都不指，函数调用后它指向新数组
    int* scores = nullptr;
    buildScores(&scores, 4);           // 传的是 scores 的地址
    for (int i = 0; i < 4; ++i) {
        std::printf("%d ", scores[i]);
    }
    std::printf("\n");                 // 10 20 30 40

    delete[] scores;                   // 谁申请谁释放（本章约定）
    scores = nullptr;
    return 0;
}
