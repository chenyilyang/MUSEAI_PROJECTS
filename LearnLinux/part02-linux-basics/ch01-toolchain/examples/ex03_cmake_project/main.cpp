// main.cpp —— ex03：三个模块拼成的多文件工程
#include <cstdio>
#include "calc.h"
#include "greet.h"

int main() {
    greet("cockpit");
    std::printf("add(3, 4) = %d\n", calc_add(3, 4));
    std::printf("mul(3, 4) = %d\n", calc_mul(3, 4));
    return 0;
}
