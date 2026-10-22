// ex01_func_template.cpp —— 函数模板与类型推导入门
// 编译基线：C++11
#include <cstdio>
#include <string>

// 一个模板顶掉一打重载：类型 T 由编译器在调用处推导，并为每个用到的
// 类型"照着模板生成一份真正的函数"（这个过程叫实例化）。
template <typename T>
T maxValue(T a, T b) {
    return a > b ? a : b;
}

int main() {
    std::printf("int:    %d\n", maxValue(3, 7));                 // T = int
    std::printf("double: %f\n", maxValue(2.5, 1.5));             // T = double

    // 字典序比较：字符串模板同样适用
    std::string s = maxValue(std::string("can"), std::string("dds"));
    std::printf("string: %s\n", s.c_str());                      // dds

    // 两个实参类型不一致时，推导会冲突（下面这行取消注释会编译报错）：
    // maxValue(3, 7.5);   // int 还是 double？编译器不替你猜
    // 解法：显式指定 T，剩下的实参按 T 转换
    std::printf("mixed:  %f\n", maxValue<double>(3, 7.5));      // 7.5
    return 0;
}
