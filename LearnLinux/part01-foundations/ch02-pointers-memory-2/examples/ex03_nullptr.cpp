// ex03_nullptr.cpp —— nullptr 与 NULL 的区别（C++11）
// 编译基线：C++11
#include <cstdio>

void f(int)  { std::printf("f(int) called\n"); }
void f(int*) { std::printf("f(int*) called\n"); }

int main() {
    f(0);          // 整数 0，只能是 int 重载
    // f(NULL);    // 不要这样写：NULL 在 C++ 里通常就是整数 0（或 0L），
                   // 会命中 f(int) 甚至产生二义性，完全不是你想要的"空指针"
    f(nullptr);    // nullptr 是专门的空指针字面量，只会命中指针重载

    int* p = nullptr;              // 指针不指向任何东西时，用 nullptr 明确表示
    if (p == nullptr) {
        std::printf("p is null, do not dereference it\n");
    }
    return 0;
}
