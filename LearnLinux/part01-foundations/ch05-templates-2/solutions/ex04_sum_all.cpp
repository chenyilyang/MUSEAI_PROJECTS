// 练习 4 参考答案：可变参数模板递归求和（C++11 写法）
#include <cstdio>

// 递归终点：没有参数时和为 0
int sumAll() {
    return 0;
}

// 每次剥掉一个参数：首元素 + 其余的和。
// 本练习限定同为 int；混合类型求和要处理返回值类型推导，
// C++17 的折叠表达式会把这件事大幅简化，专题章再讲。
template <typename... Args>
int sumAll(int first, Args... rest) {
    return first + sumAll(rest...);
}

int main() {
    std::printf("sumAll(1,2,3,4) = %d\n", sumAll(1, 2, 3, 4));   // 10
    std::printf("sumAll(7) = %d\n", sumAll(7));                  // 7
    std::printf("sumAll() = %d\n", sumAll());                    // 0
    return 0;
}
