// 练习 3 参考答案：给 typeName 增加 int 的全特化
#include <cstdio>

template <typename T>
const char* typeName(T) {
    return "generic type";
}

template <>
const char* typeName<int>(int) {          // 全特化：T = int
    return "integer";
}

template <>
const char* typeName<const char*>(const char*) {
    return "C string (const char*)";
}

int main() {
    std::printf("int:    %s\n", typeName(42));      // integer（命中新特化）
    std::printf("double: %s\n", typeName(4.2));     // generic type
    std::printf("string: %s\n", typeName("dds"));   // C string (const char*)
    return 0;
}
