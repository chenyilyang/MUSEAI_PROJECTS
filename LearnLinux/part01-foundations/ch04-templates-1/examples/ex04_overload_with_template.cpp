// ex04_overload_with_template.cpp —— 普通函数与模板同名时的决议顺序
// 规则速记：同等匹配程度下，普通函数优先；模板若能精确推导、
// 而普通函数需要类型转换，则选模板。用 show<> 可强制走模板。
// 编译基线：C++11
#include <cstdio>
#include <typeinfo>

void show(int x) {
    std::printf("ordinary function (int), value=%d\n", x);
}

template <typename T>
void show(T x) {
    std::printf("template function, T=%s\n", typeid(T).name());
    (void)x;
}

int main() {
    show(5);        // 两边都精确匹配 int → 普通函数优先
    show(5.5);      // 模板精确推导出 double；普通函数要把 double 转成 int → 选模板
    show("hi");     // const char* 只能走模板
    show<>(5);      // <> 显式要求模板：即使有普通函数也走模板（T=int）
    return 0;
}
