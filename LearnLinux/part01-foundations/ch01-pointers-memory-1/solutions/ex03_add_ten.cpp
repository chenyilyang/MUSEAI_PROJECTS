// 练习 3 参考答案：写函数 addTen，通过指针把所指的值加 10
#include <cstdio>

void addTen(int* p) {
    *p += 10;   // 顺着地址找到变量，把它加 10
}

int main() {
    int speed = 80;              // 比如车速信号值
    std::printf("before: %d\n", speed);
    addTen(&speed);              // 把 speed 的地址交给函数
    std::printf("after:  %d\n", speed);
    return 0;
}
