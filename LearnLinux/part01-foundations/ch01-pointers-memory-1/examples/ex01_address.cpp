// ex01_address.cpp —— 变量的地址与指针变量的大小
// 编译基线：C++11
#include <cstdio>

int main() {
    int    a = 42;
    double b = 3.14;
    char   c = 'X';

    // & 是"取地址"运算符：&a 得到变量 a 在内存中的地址
    std::printf("a: value=%d,  address=%p\n", a, (void*)&a);
    std::printf("b: value=%f,  address=%p\n", b, (void*)&b);
    std::printf("c: value=%c,  address=%p\n", c, (void*)&c);

    // 指针变量专门用来存放地址；指针的类型必须与所指变量的类型一致
    int*    pa = &a;
    double* pb = &b;
    char*   pc = &c;
    std::printf("pa holds address: %p\n", (void*)pa);
    std::printf("pb holds address: %p\n", (void*)pb);
    std::printf("pc holds address: %p\n", (void*)pc);

    // 关键结论：无论指向什么类型，指针变量本身的大小都相同
    // （64 位系统上是 8 字节）——因为它存的只是一个地址编号。
    std::printf("sizeof(int*)    = %zu\n", sizeof(pa));
    std::printf("sizeof(double*) = %zu\n", sizeof(pb));
    std::printf("sizeof(char*)   = %zu\n", sizeof(pc));
    return 0;
}
