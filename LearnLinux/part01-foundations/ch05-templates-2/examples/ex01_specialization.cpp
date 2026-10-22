// ex01_specialization.cpp —— 全特化：给某个具体类型开小灶
// 编译基线：C++11
#include <cstdio>

// 通用版：对绝大多数类型都返回同一句
template <typename T>
const char* typeName(T) {
    return "generic type";
}

// 全特化：当 T 恰好是 const char* 时，用这一份实现。
// 语法要点：template <> 开头，特化的签名必须与主模板在该 T 下的签名完全一致。
template <>
const char* typeName<const char*>(const char*) {
    return "C string (const char*)";
}

int main() {
    std::printf("int:    %s\n", typeName(42));        // generic type
    std::printf("double: %s\n", typeName(3.14));      // generic type
    std::printf("string: %s\n", typeName("can"));     // C string (const char*)
    return 0;
}
