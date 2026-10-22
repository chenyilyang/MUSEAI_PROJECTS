// ex02_unique_ptr.cpp —— unique_ptr：独占所有权，只能移动、不能拷贝
// 编译基线：C++11
#include <cassert>
#include <cstdio>
#include <memory>

struct Sensor {
    int id;
    explicit Sensor(int i) : id(i) { std::printf("Sensor %d created\n", id); }
    ~Sensor() { std::printf("Sensor %d destroyed\n", id); }
};

// 工厂函数：按值返回 unique_ptr（返回的是所有权本身）
std::unique_ptr<Sensor> makeSensor(int id) {
    return std::unique_ptr<Sensor>(new Sensor(id));
}

int main() {
    std::unique_ptr<Sensor> p1(new Sensor(1));

    // 转移所有权：p1 让出，p2 接手；转移后 p1 为空
    std::unique_ptr<Sensor> p2 = std::move(p1);
    assert(p1.get() == NULL);
    assert(p2.get() != NULL);
    std::printf("after move: p1 empty, p2 owns Sensor %d\n", p2->id);

    // 函数返回所有权：makeSensor 造的 Sensor 由 p3 接管
    std::unique_ptr<Sensor> p3 = makeSensor(2);
    std::printf("p3 owns Sensor %d\n", p3->id);

    // 数组版：unique_ptr<T[]> 负责 new T[] 的释放
    std::unique_ptr<int[]> arr(new int[3]);
    arr[0] = 10; arr[1] = 20; arr[2] = 30;
    std::printf("array: %d %d %d\n", arr[0], arr[1], arr[2]);

    // 出 main 时 p2、p3 按声明逆序自动销毁它们持有的 Sensor——再无手动 delete
    return 0;
}
