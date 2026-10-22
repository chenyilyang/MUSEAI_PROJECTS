// ex04_pointer_arith.cpp —— 用指针遍历数组：指针运算与边界
// 编译基线：C++11
#include <cstdio>

int main() {
    int arr[5] = {3, 7, 11, 18, 25};

    // end 指向"最后一个元素的下一个位置"（尾后指针）：
    // 它是合法的地址，可以用来比较，但绝不能解引用。
    int* end = arr + 5;

    int sum = 0;
    for (int* p = arr; p != end; ++p) {  // ++p 前进一个元素（4 字节），不是 1 字节
        sum += *p;
    }
    std::printf("sum = %d\n", sum);     // 3+7+11+18+25 = 64

    // 指针每 +1，地址前进 sizeof(元素类型) 个字节
    int* p0 = arr;
    std::printf("p0     = %p\n", (void*)p0);
    std::printf("p0 + 1 = %p (advanced by %td bytes)\n",
                (void*)(p0 + 1), (char*)(p0 + 1) - (char*)p0);
    return 0;
}
