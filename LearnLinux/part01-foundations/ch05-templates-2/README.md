# 第 5 章 模板基础（二）：特化、非类型参数、可变参数与读懂报错

## 本章目标

学完这一章，你应该能做到：

- 说清全特化与偏特化各开什么样的小灶，能写出类模板的偏特化；
- 使用非类型模板参数，理解"大小成为类型的一部分"是什么意思；
- 用 C++11 的递归手法写出可变参数模板（如 `printAll`、`sumAll`）；
- 拿到一段模板报错时，能按固定套路读出"哪错了、错在哪一行"。

**前置知识**：第 4 章（函数模板、推导、类模板）。本章只用 C++11 以内的特性。

**本章文件**：示例在 `examples/`（4 个），练习在 `exercises.md`，参考答案在 `solutions/`。

---

## 1. 全特化：给某个具体类型开小灶

通用模板照顾大多数类型，但总有例外。比如第 4 章思路下的 `typeName`：一般类型回答 "generic type" 就够了，可我们希望 C 字符串能被单独认出来。这时给那个**具体类型**单独写一份实现，叫**全特化**——所有模板参数都定死（`ex01_specialization.cpp`）：

```cpp
template <typename T>
const char* typeName(T) { return "generic type"; }

template <>                                        // 全特化的标记
const char* typeName<const char*>(const char*) {   // T 定死为 const char*
    return "C string (const char*)";
}
```

实测输出，两个分支清清楚楚：

```
int:    generic type
double: generic type
string: C string (const char*)
```

三条使用纪律：

1. **特化必须写在主模板声明之后**，且要在任何"会用到该特化的调用"之前——编译器按从上到下的顺序认识它们。
2. **特化的签名必须与主模板在该类型下推出的签名完全一致**，差一点就是另一个函数（第 5 节的报错就栽在这）。
3. 函数模板的全特化有个著名的坑：它不参与重载决议，行为有时反直觉。社区的通行建议是——**函数想给特定类型开小灶，优先用重载；特化主要留给类模板用**（下一节）。本章用函数特化只为讲清概念，工程里请优先想起第 4 章的重载决议。

## 2. 偏特化：给"一类类型"开小灶（仅类模板）

全特化定死一个类型；**偏特化定死一类类型**，比如"所有指针类型"。只有类模板能偏特化。`ex02_partial_spec.cpp` 的 `Box` 有两个版本：

```cpp
template <typename T>
class Box { /* 装普通值 */ };

template <typename T>
class Box<T*> { /* 只要装的是指针，就用我这份；成员可以与主模板不同 */ };
```

偏特化版多了一个 `deref()`——因为它确信自己装的是指针。实测：

```
Box<T> generic, get=7
Box<T*> pointer specialization, deref=8
x after = 9
```

匹配规则一句话：**编译器在所有版本里挑"最具体"的那个**。`Box<int*>` 同时能套主模板（T=int*）和偏特化（T=int + 指针模式），偏特化更具体，胜出。偏特化是库代码里的常客：标准库里大量"对指针、对引用、对数组分别处理"的类型工具都是这么搭的——第 6 章之后我们会在更深的主题里再遇见它，这里先把匹配直觉建立起来。

## 3. 非类型模板参数：把数值也当参数

模板参数不只有类型，还可以是**编译期常量值**，最常见的是 int。`ex03_nontype_param.cpp` 的定长数组：

```cpp
template <typename T, int N>
class FixedArray {
    T data_[N];
    ...
};
FixedArray<int, 4> fa;   // 4 个 int 的数组类型
FixedArray<int, 8> fb;   // 与上面是两个不同的类型！
```

"N 成为类型的一部分"是关键推论：`FixedArray<int,4>` 和 `FixedArray<int,8>` 互不兼容、不能互相赋值——大小信息被编译器写进了类型系统，于是一整类"长度对不上"的错误能在**编译期**被拦住。示例里还用编译期断言钉了一次：

```cpp
static_assert(FixedArray<int, 4>::static_size == 4, "size must be 4");
```

`static_assert` 是 C++11 的编译期断言：条件不成立直接编译失败并打出你写的消息。实测两组求和都对：`FixedArray<int,4>` 是 10，`FixedArray<double,3>` 是 1.5。想一想 CAN 报文的场景：8 字节标准帧与 64 字节 CAN FD 帧若各自是 `FixedArray<uint8_t, 8>` 与 `FixedArray<uint8_t, 64>`，把 FD 的帧塞给只接受标准帧的函数，编译当场就会报错——这种"用类型防呆"正是非类型参数的看家本领。

## 4. 可变参数模板：参数个数也可以是模板

日志、打印这类函数的参数个数天然不定。C++11 的**可变参数模板**用"参数包"接住任意多个参数，再用**递归**一个个吃掉（`ex04_variadic_print.cpp`）：

```cpp
void printAll() {                       // 终点：没参数了，收尾
    std::cout << "\n";
}

template <typename T, typename... Args>
void printAll(T first, Args... rest) {  // 每次处理一个，剩下的打包传下去
    std::cout << first << ' ';
    printAll(rest...);
}

printAll(1, 2.5, "can", 'x');   // → 1 2.5 can x
printAll("only", 2);            // → only 2
```

套路固定：**一个处理首参数的模板 + 一个空参数的终点重载**。`Args...` 是类型包、`rest...` 是把它展开传下去。记住这个骨架，本章练习的递归求和、乃至后面日志库的可变参数接口都是它。先打个预防针：这套递归写法在参数类型混杂、返回值也要推导时有点绕；C++17 的折叠表达式能把这类代码大幅简化，专题章会回来收拾它——现在把 C++11 的地基打牢，折叠表达式才有地方落脚。

顺带预告一个概念：**类型萃取（type traits）**——在编译期"查询"类型的属性（是不是指针、多大、能不能拷贝），标准库 `<type_traits>` 里全是这类工具，长相就是本章特化/偏特化的规模化应用。本章不展开，只要你认得这个词和它的大致用途，后面遇到 `std::is_pointer<T>` 一类东西不会陌生。

## 5. 读懂模板报错：一个固定套路

模板报错臭名昭著，但其实有章可循。我们故意把 `ex01` 的特化写错一处——参数误写成引用，与主模板的值传参对不上：

```cpp
template <>
const char* typeName<const char*>(const char* const&) { return "C string"; }
```

拿它去编译，GCC 的实测输出（2026-10-22 截取，已去掉文件名噪音）：

```
err1.cpp:7:13: error: template-id 'typeName<const char*>' for
  'const char* typeName(const char* const&)' does not match any template declaration
err1.cpp:3:13: note: candidate is: 'template<class T> const char* typeName(T)'
```

读法套路，三步：

1. **先只看第一条 `error:`**。它已经把结论说全了：你写的这个特化"与任何模板声明都不匹配（does not match any template declaration）"。
2. **顺着 `note:` 找对照物**。编译器把主模板的声明指给你看——把两处签名并排比：主模板在 T=const char* 时参数是 `const char*`（值），我写的是 `const char* const&`（引用），差在这里。
3. **最后才看调用点**（这次没有，因为错误在声明本身）。以后遇到长串的 "In instantiation of … required from here" 时反过来记：**链条的最底部是你的调用点，往上是层层实例化，最顶部那行 error 才是病根**。不要被行数吓住，真正要读的通常就三行：第一条 error、相关的 note、你的调用行。

养成习惯：报错再长，也先用这三步定位。模板报错的"长"大多是把实例化链条全摊开了，不等于"难"。

---

## 示例代码：构建与运行

```bash
cd examples
cmake -S . -B build
cmake --build build
./build/ex01_specialization
./build/ex02_partial_spec
./build/ex03_nontype_param
./build/ex04_variadic_print
```

核对要点：

- `ex01`：int、double 输出 generic，字符串字面量输出 C string 特化版。
- `ex02`：`Box<int>` 命中主模板；`Box<int*>` 命中偏特化，`deref()` 改值后 x 变 9。
- `ex03`：`static_assert` 编译通过；两组和为 10 与 1.500000。
- `ex04`：两行输出分别为 `1 2.5 can x` 与 `only 2`。

> 验证记录：以上结果与第 5 节报错均由作者在 Ubuntu 24.04 / GCC 13.3 环境下以 `-std=c++11 -Wall -Wextra` 实测，日期 2026-10-22。`typeid` 未在本章使用；报错文本为 GCC 原文节选。

## 动手练习

见本章 `exercises.md`（5 道题；参考答案在 `solutions/`）。

## 自测清单

- [ ] 我能写出函数模板的全特化，并说出它的两条纪律与一个著名的坑
- [ ] 我知道偏特化只有类模板能用，能预测 `Box<int*>` 这类命中结果
- [ ] 我能解释"大小成为类型的一部分"为什么能在编译期防呆
- [ ] 我能默写可变参数模板的递归骨架（首参数 + 参数包 + 终点重载）
- [ ] 给我一段模板报错，我会先看第一条 error、再看 note、最后定位调用点

## 本章小结与下一章预告

模板部分到此收官：推导、类模板、特化、非类型参数、可变参数、读报错——看库代码的"模板恐惧症"应该已经消掉大半。**下一章（RAII、智能指针与移动语义）**回到内存主题，用 C++11 的正解收拾第 2 章的事故现场：所有权写进类型、释放自动发生、拷贝变移动。这是第一部分最后一章，之后就是阶段项目 DynArray 了。
