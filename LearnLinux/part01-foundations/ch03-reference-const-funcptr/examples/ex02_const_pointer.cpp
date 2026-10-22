// ex02_const_pointer.cpp —— const 与指针的四种组合
// 读法：从右往左读声明。const 在 * 左边 = 锁"所指的值"（底层 const）；
//       const 在 * 右边 = 锁"指针本身的指向"（顶层 const）。
// 编译基线：C++11
#include <cstdio>

int main() {
    int x = 1, y = 2;

    // 1) 普通指针：指向能改，所指的值也能改
    int* p1 = &x;
    *p1 = 10;                 // x = 10
    p1 = &y;                  // 改指向，没问题

    // 2) 指向常量的指针（底层 const）：不能通过它改值，但可以改指向
    const int* p2 = &x;
    // *p2 = 20;              // 错误：read-only 位置（取消注释看报错）
    p2 = &y;                  // 改指向，可以

    // 3) 常量指针（顶层 const）：不能改指向，但可以通过它改值
    int* const p3 = &x;
    *p3 = 30;                 // x = 30
    // p3 = &y;               // 错误：不能给 const 变量赋值

    // 4) 两者皆 const：指向和值都锁死
    const int* const p4 = &x;
    // *p4 = 40;              // 错误
    // p4 = &y;               // 错误

    std::printf("x = %d, y = %d\n", x, y);   // 30, 2
    std::printf("*p2 = %d, *p4 = %d\n", *p2, *p4);  // 2, 30（读都没问题）
    return 0;
}
