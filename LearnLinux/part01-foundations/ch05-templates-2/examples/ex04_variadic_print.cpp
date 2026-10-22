// ex04_variadic_print.cpp —— 可变参数模板（C++11）：参数个数也可变
// 手法：递归展开。写一个"吃掉第一个参数"的模板，外加一个空参数的重载收尾。
// 编译基线：C++11
#include <iostream>

// 递归终点：参数吃完了，只剩换行
void printAll() {
    std::cout << "\n";
}

// Args... 叫"参数包"，rest... 是把剩下的参数打包传递
template <typename T, typename... Args>
void printAll(T first, Args... rest) {
    std::cout << first << ' ';
    printAll(rest...);          // 每次少吃一个，最终落到空参数重载
}

int main() {
    printAll(1, 2.5, "can", 'x');       // int, double, const char*, char 混着来
    printAll("only", 2);                // 个数也随意
    return 0;
}
