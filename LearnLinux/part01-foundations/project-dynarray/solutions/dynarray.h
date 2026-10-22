// dynarray.h —— DynArray 参考实现
// 验收： test_dynarray.cpp 全部断言通过；
//       ASan+UBSan 构建运行无任何 sanitizer 报告。
#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <cstddef>
#include <utility>

template <typename T>
class DynArray {
public:
    DynArray() : data_(NULL), size_(0), capacity_(0) {}

    ~DynArray() { delete[] data_; }

    // 深拷贝构造：照对方的 size 分配，逐个元素复制
    DynArray(const DynArray& other)
        : data_(other.size_ > 0 ? new T[other.size_] : NULL),
          size_(other.size_),
          capacity_(other.size_) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
    }

    // 拷贝赋值：先备好新存储再换上——自赋值天然安全，也不怕中途失败
    DynArray& operator=(const DynArray& other) {
        if (this == &other) return *this;
        T* fresh = other.size_ > 0 ? new T[other.size_] : NULL;
        for (std::size_t i = 0; i < other.size_; ++i) fresh[i] = other.data_[i];
        delete[] data_;
        data_ = fresh;
        size_ = other.size_;
        capacity_ = other.size_;
        return *this;
    }

    // 移动构造：过户指针三件套——接管、源置空
    DynArray(DynArray&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = NULL;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // 移动赋值：释放自己的旧存储，再过户
    DynArray& operator=(DynArray&& other) noexcept {
        if (this == &other) return *this;
        delete[] data_;
        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.data_ = NULL;
        other.size_ = 0;
        other.capacity_ = 0;
        return *this;
    }

    void push_back(const T& value) {
        if (size_ == capacity_) grow();
        data_[size_] = value;
        ++size_;
    }

    T& operator[](std::size_t i) { return data_[i]; }
    const T& operator[](std::size_t i) const { return data_[i]; }

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }

private:
    // 容量翻倍：0→1，之后每次 ×2。搬运时对元素用 std::move——
    // 容器自己也享受第 6 章的移动红利（元素不支持移动时会自然退回赋值拷贝的效果，
    // 对本项目的 int / std::string 都正确）。
    void grow() {
        std::size_t newCap = capacity_ == 0 ? 1 : capacity_ * 2;
        T* fresh = new T[newCap];
        for (std::size_t i = 0; i < size_; ++i) fresh[i] = std::move(data_[i]);
        delete[] data_;
        data_ = fresh;
        capacity_ = newCap;
    }

    T* data_;
    std::size_t size_;
    std::size_t capacity_;
};

#endif  // DYNARRAY_H
