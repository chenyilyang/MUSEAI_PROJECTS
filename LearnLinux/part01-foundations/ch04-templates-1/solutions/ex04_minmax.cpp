// 练习 4 参考答案：类模板 MinMax，存两个值并提供大小比较
#include <cstdio>

template <typename T>
class MinMax {
public:
    MinMax(const T& a, const T& b) : a_(a), b_(b) {}
    T getMin() const { return a_ < b_ ? a_ : b_; }
    T getMax() const { return a_ > b_ ? a_ : b_; }
private:
    T a_;
    T b_;
};

int main() {
    MinMax<int> mm1(17, 42);
    std::printf("int: min=%d, max=%d\n", mm1.getMin(), mm1.getMax());     // 17, 42

    MinMax<double> mm2(3.5, 2.25);
    std::printf("double: min=%f, max=%f\n", mm2.getMin(), mm2.getMax());  // 2.25, 3.5
    return 0;
}
