// ex04_move_semantics.cpp —— 拷贝 vs 移动：用计数器把开销看出来
// Buffer 管理一块堆数组。拷贝 = 再分配一块、逐元素搬；
// 移动 = 只把指针过户，源对象置空——大对象时两者成本天差地别。
// 编译基线：C++11
#include <cstddef>
#include <cstdio>
#include <utility>
#include <vector>

class Buffer {
public:
    explicit Buffer(std::size_t n) : data_(new int[n]), size_(n) {
        for (std::size_t i = 0; i < n; ++i) data_[i] = 0;
    }
    // 拷贝构造：深拷贝
    Buffer(const Buffer& other) : data_(new int[other.size_]), size_(other.size_) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
        ++copies;
    }
    // 移动构造：偷指针，源置空。noexcept 很关键——见正文
    Buffer(Buffer&& other) noexcept : data_(other.data_), size_(other.size_) {
        other.data_ = NULL;
        other.size_ = 0;
        ++moves;
    }
    ~Buffer() { delete[] data_; }

    std::size_t size() const { return size_; }

    static int copies;
    static int moves;
    static void resetCounters() { copies = 0; moves = 0; }
private:
    int* data_;
    std::size_t size_;
    Buffer& operator=(const Buffer&);      // 本章聚焦构造，赋值先禁掉
    Buffer& operator=(Buffer&&);
};
int Buffer::copies = 0;
int Buffer::moves = 0;

// 按值传参：参数的构造方式由调用方给的是左值还是右值决定
void consume(Buffer b) {
    std::printf("  consume got buffer of size %zu\n", b.size());
}

int main() {
    // —— 第一幕：左值触发拷贝，std::move 之后触发移动 ——
    Buffer a(4);
    Buffer b = a;                    // 左值 → 拷贝构造
    std::printf("copies=%d moves=%d (expect 1, 0)\n", Buffer::copies, Buffer::moves);
    Buffer c = std::move(a);         // std::move 把 a 标记为"可搬走" → 移动构造
    std::printf("copies=%d moves=%d (expect 1, 1); a.size=%zu (moved-from)\n",
                Buffer::copies, Buffer::moves, a.size());

    // —— 第二幕：按值传参时的差别 ——
    Buffer::resetCounters();
    Buffer src(3);
    consume(src);                    // 左值 → 参数拷贝构造
    std::printf("after consume(lvalue): copies=%d moves=%d (expect 1, 0)\n",
                Buffer::copies, Buffer::moves);
    consume(std::move(src));         // 右值 → 参数移动构造
    std::printf("after consume(rvalue): copies=%d moves=%d (expect 1, 1)\n",
                Buffer::copies, Buffer::moves);

    // —— 第三幕：vector 扩容——noexcept 移动的用武之地 ——
    Buffer::resetCounters();
    std::vector<Buffer> v;
    v.push_back(Buffer(1));
    v.push_back(Buffer(1));
    v.push_back(Buffer(1));
    // 3 个临时对象各移动一次入仓；扩容两次（容量 1→2→4）再各搬一次已存元素：
    // 移动合计 6 次，拷贝 0 次。前提是移动构造带 noexcept——
    // 若去掉 noexcept，vector 为保证异常安全会改走拷贝，计数立刻变样，可亲手试。
    std::printf("vector growth: copies=%d moves=%d (expect 0, 6)\n",
                Buffer::copies, Buffer::moves);
    return 0;
}
