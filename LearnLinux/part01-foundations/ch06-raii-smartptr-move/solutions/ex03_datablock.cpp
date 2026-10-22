// 练习 3 参考答案：unique_ptr 的工厂函数与所有权转移
#include <cassert>
#include <cstdio>
#include <memory>

struct DataBlock {
    int value;
    explicit DataBlock(int v) : value(v) { std::printf("DataBlock %d created\n", value); }
    ~DataBlock() { std::printf("DataBlock %d destroyed\n", value); }
};

std::unique_ptr<DataBlock> createBlock(int v) {
    return std::unique_ptr<DataBlock>(new DataBlock(v));
}

int main() {
    std::unique_ptr<DataBlock> owner = createBlock(77);
    std::printf("owner holds value %d\n", owner->value);

    std::unique_ptr<DataBlock> next = std::move(owner);   // 所有权过户
    assert(owner.get() == NULL);
    std::printf("after transfer: next holds value %d\n", next->value);
    return 0;   // next 出作用域，DataBlock 自动销毁（看 destroyed 日志）
}
