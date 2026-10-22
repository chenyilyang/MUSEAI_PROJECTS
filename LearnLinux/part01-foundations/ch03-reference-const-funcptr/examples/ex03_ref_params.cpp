// ex03_ref_params.cpp —— 传参三式：指针、引用、const 引用
// 编译基线：C++11
#include <cstdio>

void swapByPtr(int* a, int* b) {          // 指针版：调用处要取地址，函数里要解引用
    int t = *a; *a = *b; *b = t;
}

void swapByRef(int& a, int& b) {          // 引用版：调用处和函数里都像用变量本身
    int t = a; a = b; b = t;
}

// 一个带拷贝计数的大对象，用来对比"按值传参"与"const 引用传参"的成本
struct Big {
    int v;
    static int copies;
    Big(int x = 0) : v(x) {}
    Big(const Big& other) : v(other.v) { ++copies; }
};
int Big::copies = 0;

void byValue(Big b)    { std::printf("byValue got %d\n", b.v); }
void byConstRef(const Big& b) { std::printf("byConstRef got %d\n", b.v); }

int main() {
    int a = 1, b = 2;
    swapByPtr(&a, &b);
    std::printf("after swapByPtr: a=%d, b=%d\n", a, b);   // 2, 1
    swapByRef(a, b);
    std::printf("after swapByRef: a=%d, b=%d\n", a, b);   // 1, 2（换回来了）

    Big big(7);
    Big::copies = 0;
    byValue(big);                          // 按值：发生 1 次拷贝
    std::printf("copies after byValue   = %d\n", Big::copies);   // 1
    byConstRef(big);                       // const 引用：0 次拷贝
    std::printf("copies after byConstRef = %d\n", Big::copies);  // 还是 1
    return 0;
}
