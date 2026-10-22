// ex03_pointer_and_array.cpp —— 指针与数组的关系
// 编译基线：C++11
#include <cstdio>

int main() {
    int arr[5] = {10, 20, 30, 40, 50};

    // 数组名在表达式中会"退化"为指向首元素的指针：arr == &arr[0]
    std::printf("arr     = %p\n", (void*)arr);
    std::printf("&arr[0] = %p\n", (void*)&arr[0]);

    // 每个元素的地址：相邻元素相差 sizeof(int) = 4 字节
    for (int i = 0; i < 5; ++i) {
        std::printf("&arr[%d] = %p, arr[%d] = %d\n",
                    i, (void*)&arr[i], i, arr[i]);
    }

    // 指针相减得到的是"元素个数"，不是字节数
    std::printf("(&arr[4] - &arr[0]) = %td elements\n", &arr[4] - &arr[0]);

    // arr[i] 与 *(arr + i) 完全等价
    std::printf("arr[2] = %d, *(arr + 2) = %d\n", arr[2], *(arr + 2));
    return 0;
}
