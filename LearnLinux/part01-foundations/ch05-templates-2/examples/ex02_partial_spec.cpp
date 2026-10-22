// ex02_partial_spec.cpp —— 偏特化：给"一类类型"开小灶（类模板才有）
// 编译基线：C++11
#include <cstdio>

// 主模板：Box 装一个普通值
template <typename T>
class Box {
public:
    explicit Box(const T& v) : v_(v) {}
    T get() const { return v_; }
    const char* kind() const { return "Box<T> generic"; }
private:
    T v_;
};

// 偏特化：Box<T*> —— 只要装的是指针，就用这一份。
// 它可以有与主模板不同的成员：这里多了一个 deref()。
template <typename T>
class Box<T*> {
public:
    explicit Box(T* p) : p_(p) {}
    T& deref() const { return *p_; }
    const char* kind() const { return "Box<T*> pointer specialization"; }
private:
    T* p_;
};

int main() {
    Box<int> b1(7);
    std::printf("%s, get=%d\n", b1.kind(), b1.get());       // generic, 7

    int x = 8;
    Box<int*> b2(&x);                                        // 命中偏特化
    std::printf("%s, deref=%d\n", b2.kind(), b2.deref());    // pointer spec, 8

    b2.deref() = 9;                                          // 通过 deref 改到 x 本身
    std::printf("x after = %d\n", x);                        // 9
    return 0;
}
