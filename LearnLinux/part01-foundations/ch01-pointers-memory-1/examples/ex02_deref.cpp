// ex02_deref.cpp —— 解引用：通过指针读写变量
// 编译基线：C++11
#include <cstdio>

int main() {
    int score = 60;
    int* p = &score;   // p 指向 score

    std::printf("before: score = %d\n", score);

    // *p 是"解引用"：顺着地址找到那间房，取出/放入里面的值
    std::printf("read via *p: %d\n", *p);

    *p = 95;           // 通过指针改值，改的就是 score 本身

    std::printf("after:  score = %d\n", score);
    return 0;
}
