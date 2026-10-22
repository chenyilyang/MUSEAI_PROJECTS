// dynarray.h —— 阶段项目起始骨架
// 接口已声明完整；标 TODO 的函数体是空壳，验收程序现在必然失败。
// 你的任务：只改这个文件，把 TODO 逐个实现，直到验收全部通过。
// 约束：元素存储必须自己用 new T[] / delete[] 管理，不许内部套 std::vector。
#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <cstddef>

template <typename T>
class DynArray {
public:
    DynArray() : data_(NULL), size_(0), capacity_(0) {}

    ~DynArray() {
        // TODO: 释放 data_ 指向的数组（回想第 2、6 章）
    }

    // 拷贝构造：深拷贝——新分配一份存储，逐个复制元素
    DynArray(const DynArray& other) : data_(NULL), size_(0), capacity_(0) {
        // TODO
        (void)other;
    }

    // 拷贝赋值：注意自赋值（a = a）也要安全
    DynArray& operator=(const DynArray& other) {
        // TODO
        (void)other;
        return *this;
    }

    // 移动构造：接管对方的存储，对方置空。记得 noexcept（第 6 章）
    DynArray(DynArray&& other) noexcept : data_(NULL), size_(0), capacity_(0) {
        // TODO
        (void)other;
    }

    // 移动赋值：先释放自己现有的存储，再接管对方
    DynArray& operator=(DynArray&& other) noexcept {
        // TODO
        (void)other;
        return *this;
    }

    // 尾部追加。容量不足时翻倍增长：0→1→2→4→8…
    void push_back(const T& value) {
        // TODO
        (void)value;
    }

    T& operator[](std::size_t i) { return data_[i]; }
    const T& operator[](std::size_t i) const { return data_[i]; }

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

    // 迭代器：裸指针即可（第 1 章），范围 for 靠这两个函数工作
    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }

private:
    T* data_;
    std::size_t size_;
    std::size_t capacity_;
};

#endif  // DYNARRAY_H
