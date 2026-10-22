// main.cpp —— ex02 的主程序，只认 calc.h，不关心库是静态还是动态
#include <cstdio>
#include "calc.h"

int main() {
    std::printf("add(3, 4) = %d\n", calc_add(3, 4));   // 7
    std::printf("mul(3, 4) = %d\n", calc_mul(3, 4));   // 12
    return 0;
}
