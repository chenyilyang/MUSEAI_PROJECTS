// 练习 4 参考答案：给 SimpleBuffer 补上移动构造，并验证开销变化
#include <cstddef>
#include <cstdio>
#include <utility>

class SimpleBuffer {
public:
    explicit SimpleBuffer(std::size_t n) : data_(new int[n]), size_(n) {}
    SimpleBuffer(const SimpleBuffer& other)
        : data_(new int[other.size_]), size_(other.size_) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
        ++copies;
    }
    // 新增：移动构造——过户指针，源置空
    SimpleBuffer(SimpleBuffer&& other) noexcept
        : data_(other.data_), size_(other.size_) {
        other.data_ = NULL;
        other.size_ = 0;
        ++moves;
    }
    ~SimpleBuffer() { delete[] data_; }
    std::size_t size() const { return size_; }

    static int copies;
    static int moves;
private:
    int* data_;
    std::size_t size_;
    SimpleBuffer& operator=(const SimpleBuffer&);
    SimpleBuffer& operator=(SimpleBuffer&&);
};
int SimpleBuffer::copies = 0;
int SimpleBuffer::moves = 0;

void consume(SimpleBuffer b) { (void)b; }

int main() {
    SimpleBuffer src(100);
    consume(src);                  // 左值：拷贝（深拷贝 100 个 int）
    consume(std::move(src));       // 右值：移动（只过户指针）
    std::printf("copies=%d moves=%d (expect 1, 1); src.size=%zu\n",
                SimpleBuffer::copies, SimpleBuffer::moves, src.size());   // src 已被搬空
    return 0;
}
