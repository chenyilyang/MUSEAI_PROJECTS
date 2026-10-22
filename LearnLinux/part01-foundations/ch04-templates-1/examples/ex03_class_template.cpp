// ex03_class_template.cpp —— 类模板：成员、类外定义、多个类型参数
// 编译基线：C++11
#include <cstdio>
#include <string>

// 两个类型参数的类模板：字段类型由使用者决定
template <typename T1, typename T2>
struct Pair {
    T1 first;
    T2 second;
    void print() const {
        std::printf("(%d, %s)\n", first, second);   // 本例中 T1=int, T2=const char*
    }
};

// 一个类型参数的类模板
template <typename T>
class Box {
public:
    Box() : value_() {}
    explicit Box(const T& v) : value_(v) {}
    void set(const T& v) { value_ = v; }   // 成员函数在类内定义
    T get() const;                          // 只声明，类外定义（见下）
private:
    T value_;
};

// 类外定义成员函数：必须带上 template 前缀和 Box<T>:: 作用域
template <typename T>
T Box<T>::get() const {
    return value_;
}

int main() {
    Pair<int, const char*> signal;
    signal.first = 2500;
    signal.second = "rpm";
    signal.print();                          // (2500, rpm)

    Box<int> bi;
    bi.set(42);
    std::printf("Box<int> = %d\n", bi.get());          // 42

    Box<std::string> bs(std::string("SOME/IP"));
    std::printf("Box<string> = %s\n", bs.get().c_str());   // SOME/IP
    return 0;
}
