// ex03_nontype_param.cpp —— 非类型模板参数：把数值也当作模板参数
// 编译基线：C++11
#include <cstdio>

// N 不是类型，而是一个 int 值，在编译期就定死，
// 于是数组大小成为类型的一部分：FixedArray<int,4> 与 FixedArray<int,8>
// 是两个完全不同的类型。
template <typename T, int N>
class FixedArray {
public:
    FixedArray() {
        for (int i = 0; i < N; ++i) data_[i] = T();
    }
    T& operator[](int i) { return data_[i]; }
    const T& operator[](int i) const { return data_[i]; }
    int count() const { return N; }

    static const int static_size = N;
private:
    T data_[N];
};

// 编译期就能校验大小——写错在编译时当场被拦
static_assert(FixedArray<int, 4>::static_size == 4, "size must be 4");

int main() {
    FixedArray<int, 4> fa;
    fa[0] = 1; fa[1] = 2; fa[2] = 3; fa[3] = 4;
    int sum = 0;
    for (int i = 0; i < fa.count(); ++i) sum += fa[i];
    std::printf("FixedArray<int,4> sum = %d\n", sum);        // 10

    FixedArray<double, 3> fd;
    fd[0] = 0.5; fd[1] = 0.5; fd[2] = 0.5;
    double dsum = 0;
    for (int i = 0; i < fd.count(); ++i) dsum += fd[i];
    std::printf("FixedArray<double,3> sum = %f\n", dsum);    // 1.5
    return 0;
}
