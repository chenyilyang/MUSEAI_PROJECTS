# 第 6 章 RAII、智能指针与移动语义

## 本章目标

学完这一章，你应该能做到：

- 说清 RAII 的机制，并能亲手把一种资源（文件、内存、锁）包成 RAII 类；
- 为一段代码选对智能指针：默认 unique_ptr，确需共享才用 shared_ptr，并会用 weak_ptr 打破循环引用；
- 分清左值与右值，能亲手写拷贝构造与移动构造，用计数器验证两者的开销差别；
- 知道移动构造的 `noexcept` 为什么会影响标准容器的行为。

**前置知识**：第 1～5 章，重点是第 2 章（所有权、泄漏、重复释放的现场）。本章只用 C++11 以内的特性。

**本章文件**：示例在 `examples/`（4 个），练习在 `exercises.md`，参考答案在 `solutions/`。

---

## 1. RAII：让"释放"这件事不可能被忘记

回看第 2 章的事故清单——泄漏、悬垂——共同病因是一句话：**资源的获取与释放在代码里相隔很远，中间任何一条岔路（提前 return、出错分支）都可能让释放被跳过。**

C++ 的解法叫 **RAII**（Resource Acquisition Is Initialization）：把资源交给一个**栈对象**保管——构造函数里获取，析构函数里释放。而栈对象的析构由语言兜底：**无论从哪条路径离开作用域（正常走完、提前 return、抛异常），析构都会执行**。释放不再依赖程序员"记得写"，而是写进了类型本身。

`ex01_raii_file.cpp` 把 C 文件句柄包成了 `FileWriter`，实测输出：

```
opened: ex01_normal.txt
closed automatically
opened: ex01_early.txt
closed automatically        ← 提前 return 的那条路，照样关闭
content of ex01_normal.txt: hello raii
content of ex01_early.txt: early return
```

两个要点回味一下：其一，它顺手禁掉了拷贝（第 3 章学过的 `=delete` 前身写法或私有声明）——一个文件只能有一个保管人，这是"所有权"第一次出现在类型定义里。其二，注意这个模式不限于文件：内存、socket、锁、句柄……凡是"用完必须还"的东西都能这样包。下面三节的智能指针，本质上是标准库预制好的 RAII 类。

## 2. unique_ptr：独占所有权，是默认值

`unique_ptr` 独占它指向的对象：不能拷贝（所有权无二主），只能用 `std::move` **转移**；自己被销毁时，所指对象一并销毁。

`ex02_unique_ptr.cpp` 的实测序列把语义演全了：

```
Sensor 1 created
after move: p1 empty, p2 owns Sensor 1     ← 转移后源为空，程序里用 assert 钉死
Sensor 2 created
p3 owns Sensor 2                            ← 工厂函数按值返回，所有权随之交付
array: 10 20 30                             ← unique_ptr<T[]> 管数组
Sensor 2 destroyed                          ← 出作用域自动销毁，无手动 delete
Sensor 1 destroyed
```

记住三点用法直觉：**默认选它**（零开销、无计数，比裸指针只多一份安全）；转移用 `std::move`，转移后源指针为空、可以放心检查；函数"生产"对象时按值返回 `unique_ptr` 是标准姿势，调用方接住即拥有。第 2 章那些 `new` 出来没人管的内存，现实中的正解就是在这里被接管的。

## 3. shared_ptr 与 weak_ptr：共享所有权与它的代价

确实需要多个主人共享一个对象时，用 `shared_ptr`：内部维护**引用计数**，拷贝 +1、销毁 -1，计数归零才销毁对象。`ex03_shared_weak.cpp` 第一幕的实测：

```
count after create: 1
count after copy:   2
count after copy2:  3
count after scope:  1      ← 两个拷贝离开作用域，计数如数回落
```

但共享有个致命陷阱——**循环引用**：两个对象各持一个指向对方的 `shared_ptr`，外部引用都撤了，环里彼此还攥着对方的计数，谁也到不了 0。实测第二幕：作用域结束后两个对象**仍然活着**（`alive = 2`，泄漏已成事实）：

```
NodeShared alive after scope: 2 (leaked!)
```

解法是把环里的引用降级为 **`weak_ptr`**：它只"观察"对象，不参与计数。环断开了，对象如期销毁：

```
NodeWeak alive after scope:   0 (clean)
```

两个必备常识：`weak_ptr` 不能直接解引用，使用时靠 `lock()` 临时"提升"回 `shared_ptr`——提升成功才说明对象还活着，提升失败（返回空）说明对象已销毁。它不延长命，只提供一个**安全确认对方还在**的手段。选型口诀收尾：**先想 unique，共享才 shared，成环用 weak 断一刀**；另外 shared 的计数增减要走原子操作，它不是免费的——这也是它不该当默认值的理由。

## 4. 移动语义：拷贝是搬家，移动是过户

先把地基概念立住：**左值**是有名字、可以取地址、之后还要用的东西（如变量）；**右值**是临时的、马上就不要了的值（如函数返回值、`std::move(...)` 的结果）。区别的实际意义在于：对右值，我们可以放心地"偷"走它内部的资源，反正它转瞬即逝。

`ex04_move_semantics.cpp` 的 `Buffer` 管理一块堆数组，并给拷贝/移动构造各装了计数器。深拷贝要重新分配并逐个复制元素，移动只做一件事：把指针过户、把源置空。实测第一幕：

```
Buffer b = a;            // a 是左值 → 拷贝构造：copies=1
Buffer c = std::move(a); // std::move 把 a 标为右值 → 移动构造：moves=1，a.size 变为 0
```

两个必澄清点：**`std::move` 本身什么都不搬**，它只是一个"这东西可以搬走了"的标记（本质是类型转换），真正搬家的是移动构造函数；被搬过的对象处于"内容已转移、但仍可安全销毁/赋值"的状态，所以把源置空是移动构造的责任。第二幕里按值传参的函数 `consume` 把这一点用足了：传左值参数被拷贝（copies=1），传 `std::move(src)` 参数被移动（moves=1）——**调用方在现场决定这次传参付哪种成本**。

第三幕是本章的点睛之笔：

```
vector growth: copies=0 moves=6
```

向 `vector` 里连塞 3 个临时 `Buffer`：每次入仓一次移动（3 次），扩容两次搬运已有元素（容量 1→2 搬 1 个、2→4 搬 2 个，共 3 次），总计 6 次移动、**零拷贝**。这个结果有个前提——移动构造标了 `noexcept`。vector 扩容时若移动可能抛异常，它宁可改走拷贝来保证"搬一半出错也能回滚"。**亲手实验**：把 ex04 里移动构造的 `noexcept` 去掉再跑，计数会立刻变成拷贝主导。这就是"为什么库代码的移动构造几乎都带 noexcept"的答案，不用背，用计数看得见。

---

## 示例代码：构建与运行

```bash
cd examples
cmake -S . -B build
cmake --build build
cd build   # ex01 的数据文件生成在当前目录
./ex01_raii_file
./ex02_unique_ptr
./ex03_shared_weak
./ex04_move_semantics
```

核对要点：

- `ex01`：两条路径都打印自动关闭；随后读出的两个文件内容完整。
- `ex02`：所有 assert 通过（退出码 0）；销毁日志在 main 结束时按逆序出现。
- `ex03`：计数序列 1、2、3、1；shared 环作用域后 alive=2，weak 版为 0。
- `ex04`：三幕计数与正文完全一致，尤其第三幕 copies=0、moves=6。

> 验证记录：以上结果由作者在 Ubuntu 24.04 / GCC 13.3 环境下以 `-std=c++11 -Wall -Wextra` 构建实测（零警告），日期 2026-10-22。vector 扩容次数依标准库实现，计数以本环境（libstdc++）实测为准。

## 动手练习

见本章 `exercises.md`（5 道题；参考答案在 `solutions/`）。

## 自测清单

- [ ] 我能解释为什么 RAII 能堵住"提前 return 漏释放"，并能自己包一个 RAII 类
- [ ] 我知道何时选 unique_ptr、何时才轮到 shared_ptr，并能说出 shared 的成本
- [ ] 我能描述循环引用怎么形成，并用 weak_ptr 解开
- [ ] 我能分清左值/右值，知道 std::move 只做标记、搬家靠移动构造
- [ ] 我能写出带计数器的拷贝/移动构造，并解释 vector 扩容为何偏爱带 noexcept 的移动

## 本章小结与下一章预告

第一部分到此收官：指针、内存、引用、函数指针、模板、RAII、智能指针、移动语义——这套地基正是为读懂、写对后面的系统代码准备的。**下一步是第一部分阶段项目：动态数组 DynArray**——把模板、RAII、拷贝/移动一次性用在一个自己实现的容器上，并用带断言的验收程序检验它。这一关过了，我们正式进入第二部分：Linux 系统编程。
