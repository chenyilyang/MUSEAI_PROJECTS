// test_dynarray.cpp —— DynArray 验收程序（固定，不得修改）
// 全部断言通过并打印 ALL TESTS PASSED 才算实现达标。
#include "dynarray.h"

#include <cassert>
#include <cstdio>
#include <string>
#include <utility>

int main() {
    // —— 1. 基本存取与容量增长序列：0 → 1 → 2 → 4 → 8（翻倍）——
    DynArray<int> a;
    assert(a.size() == 0);
    assert(a.capacity() == 0);
    assert(a.empty());

    a.push_back(10);
    assert(a.size() == 1);
    assert(a.capacity() == 1);
    a.push_back(20);
    assert(a.capacity() == 2);
    a.push_back(30);
    assert(a.capacity() == 4);
    a.push_back(40);
    a.push_back(50);
    assert(a.capacity() == 8);
    assert(a.size() == 5);
    assert(!a.empty());
    assert(a[0] == 10 && a[4] == 50);

    a[0] = 11;                                   // 非 const 版 operator[] 可写
    assert(a[0] == 11);
    const DynArray<int>& ca = a;
    assert(ca[1] == 20);                         // const 版 operator[]

    // —— 2. 范围 for（依赖 begin()/end()）——
    int sum = 0;
    for (int v : a) sum += v;
    assert(sum == 11 + 20 + 30 + 40 + 50);       // 151

    // —— 3. 深拷贝：构造与赋值都要独立 ——
    DynArray<int> b = a;                         // 拷贝构造
    b[0] = 999;
    assert(a[0] == 11);                          // 原数组不受影响
    assert(b[0] == 999);

    DynArray<int> c;
    c = a;                                       // 拷贝赋值
    c[1] = 888;
    assert(a[1] == 20);
    assert(c[1] == 888);

    // —— 4. 自赋值安全 ——
    a = a;
    assert(a.size() == 5 && a[0] == 11);

    // —— 5. 移动：构造与赋值；源必须被搬空 ——
    DynArray<int> d = std::move(b);              // 移动构造
    assert(d.size() == 5);
    assert(d[0] == 999);
    assert(b.size() == 0);
    assert(b.capacity() == 0);

    DynArray<int> e;
    e = std::move(d);                            // 移动赋值
    assert(e.size() == 5 && e[0] == 999);
    assert(d.size() == 0);
    assert(d.capacity() == 0);

    e.push_back(60);                             // 移动后的目标仍可继续使用
    assert(e.size() == 6 && e[5] == 60);

    // —— 6. 非平凡元素类型：std::string 也要深拷贝成立 ——
    DynArray<std::string> s;
    s.push_back("can");
    s.push_back("someip");
    s.push_back("dds");
    assert(s.size() == 3);
    assert(s.capacity() == 4);
    assert(s[1] == "someip");

    DynArray<std::string> s2 = s;
    s2[0] = "changed";
    assert(s[0] == "can");                       // 字符串同样互不影响

    std::printf("ALL TESTS PASSED\n");
    return 0;
}
