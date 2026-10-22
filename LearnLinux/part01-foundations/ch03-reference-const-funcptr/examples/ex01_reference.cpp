// ex01_reference.cpp —— 引用：变量的别名
// 编译基线：C++11
#include <cstdio>

int main() {
    int a = 10;
    int& r = a;          // r 是 a 的别名：声明时必须初始化，之后终身绑定 a

    r = 25;              // 改 r 就是改 a
    std::printf("a = %d, r = %d\n", a, r);     // 25, 25

    int b = 99;
    r = b;               // 注意！这不是"改绑"，而是把 b 的值赋给 r（也就是 a）
    std::printf("a = %d, b = %d\n", a, b);     // 99, 99 —— r 仍然是 a 的别名

    // int& bad;         // 错误：引用声明时不初始化，编译不过（取消注释试试）

    return 0;
}
