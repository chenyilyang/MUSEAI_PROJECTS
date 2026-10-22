// ex03_shared_weak.cpp —— shared_ptr 引用计数；循环引用与 weak_ptr 破环
// 编译基线：C++11
#include <cassert>
#include <cstdio>
#include <memory>

struct NodeShared {
    std::shared_ptr<NodeShared> next;      // 强引用：参与计数
    static int alive;
    NodeShared() { ++alive; }
    ~NodeShared() { --alive; }
};
int NodeShared::alive = 0;

struct NodeWeak {
    std::weak_ptr<NodeWeak> next;          // 弱引用：只观察，不参与计数
    static int alive;
    NodeWeak() { ++alive; }
    ~NodeWeak() { --alive; }
};
int NodeWeak::alive = 0;

int main() {
    // —— 第一幕：引用计数怎么涨落 ——
    std::shared_ptr<int> a(new int(42));
    std::printf("count after create: %ld\n", a.use_count());        // 1
    {
        std::shared_ptr<int> b = a;                                  // 拷贝：计数+1
        std::printf("count after copy:   %ld\n", a.use_count());     // 2
        std::shared_ptr<int> c = a;
        std::printf("count after copy2:  %ld\n", a.use_count());     // 3
    }   // b、c 离开作用域：计数回落
    std::printf("count after scope:  %ld\n", a.use_count());         // 1

    // —— 第二幕：shared_ptr 互相指，会一起泄漏 ——
    {
        std::shared_ptr<NodeShared> x(new NodeShared);
        std::shared_ptr<NodeShared> y(new NodeShared);
        x->next = y;
        y->next = x;   // 成环：彼此都攥着对方的强引用
    }   // x、y 本身没了，但环里每个对象的计数还剩 1 —— 谁也到不了 0，析构永不发生
    std::printf("NodeShared alive after scope: %d (leaked!)\n", NodeShared::alive);  // 2

    // —— 第三幕：把环中的引用换成 weak_ptr ——
    {
        std::shared_ptr<NodeWeak> x(new NodeWeak);
        std::shared_ptr<NodeWeak> y(new NodeWeak);
        x->next = y;
        y->next = x;   // weak 不计数：x、y 离开作用域后计数归零，正常析构
    }
    std::printf("NodeWeak alive after scope:   %d (clean)\n", NodeWeak::alive);     // 0
    assert(NodeWeak::alive == 0);
    return 0;
}
