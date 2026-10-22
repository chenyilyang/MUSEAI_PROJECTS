// ex02_deduction.cpp —— 类型推导规则：按值、引用、数组退化
// typeid 的名字是编译器内部编码（i=int, Pi=int*, PKc=const char*），
// 只作对照，以正文表格为准。
// 编译基线：C++11
#include <cstdio>
#include <typeinfo>

template <typename T>
void byVal(T param) {          // 按值传参
    std::printf("byVal : T=%-4s sizeof(T)=%zu\n", typeid(T).name(), sizeof(T));
    (void)param;
}

template <typename T>
void byRef(T& param) {         // 引用传参
    std::printf("byRef : sizeof(param)=%zu\n", sizeof(param));
    (void)param;
}

int main() {
    int x = 1;
    const int cx = 2;
    int arr[4] = {0, 0, 0, 0};

    byVal(x);        // T=int，sizeof=4
    byVal(cx);       // T=int：按值推导时，实参的顶层 const 被丢弃（反正要拷贝）
    byVal(arr);      // T=int*：数组退化为指针，sizeof=8
    byVal("Linux");  // T=const char*（字符串字面量是 const char 数组，退化后带 const）

    byRef(x);        // param 是 int&，sizeof=4
    byRef(cx);       // T=const int，param 是 const int&
    byRef(arr);      // 不退化！param 是 int(&)[4]，sizeof=16

    std::printf("sizeof(arr) itself = %zu\n", sizeof(arr));   // 16，对照用
    return 0;
}
