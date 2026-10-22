# LearnLinux 教程产出计划（Implementation Plan）

> **执行约定：** 本计划按章执行。每章完成并入库后必须暂停，向学习者汇报并得到确认，才开始下一章（逐章确认制）。每章的示例代码先在作者验证环境中实际编译、运行、核对输出，再写入正文——正文里写的预期输出必须与实测一致。

**Goal:** 按设计文档产出 LearnLinux 教程的全部章节（中文 Markdown + 可运行示例），使学习者从 C/C++ 基础薄弱点出发，最终能独立完成"座舱信号网关"毕业项目。

**Architecture:** 一条主线六个部分：地基补强 → Linux 开发起步 → 并发与多线程 → 网络与 IPC → 车载专题 → 毕业项目。本计划对第一部分做逐章详细规划；第二至第六部分给出分章路线图，各部分开始前按同一模板补齐逐章细节（因学习者反馈会调整后续深浅）。

**Tech Stack:** C++（GCC 13、CMake、GDB）、SocketCAN/vcan、vsomeip 3.x、Fast DDS 3.x、Markdown + GitHub 仓库（chenyilyang/MUSEAI_PROJECTS）。

**Spec:** `LearnLinux/docs/2026-10-22-linux-cpp-tutorial-design.md`（已定稿）

## Global Constraints

- 教程语言中文、Markdown 格式，存放于仓库 `LearnLinux/` 目录。
- 工具链基线：Ubuntu 24.04、GCC 13、CMake、GDB（学习者练习环境为无限制的公司自用开发机）。
- C++ 标准：C++11 为工作基线；C++14 特性随用随讲；C++17 与 C++20 各设专题章节；C++23 点到为止。每章正文须标注本章所用特性的版本。
- 每章示例入库前，作者须在验证环境以 `-Wall -Wextra` 编译零警告并实际运行核对；并发章节示例额外过一遍 ThreadSanitizer。
- 章节固定六段结构：本章目标 → 知识讲解 → 示例代码 → 动手练习（附参考答案）→ 自测清单 → 小结与预告。
- 逐章确认制：每章入库后暂停汇报，学习者确认后才产出下一章。
- CAN 以 SocketCAN + vcan 为正规打法；Vehicle HAL 模拟为独立进程服务（Unix 域套接字），数据结构按真实 VHAL 类型形状建模；DBC 以手写信号表教解码，不实现完整解析器。
- vsomeip 3.x、Fast DDS 3.x 均源码构建，具体小版本以产出对应章节时实测通过者为准并在章节中写明。

## Review Focus（教程最容易翻车、须逐章设防的五点）

1. **输出不确定性**：地址、线程调度顺序等输出天然可变——正文只断言"关系与格式"（如地址递增、元素连续），不断言具体数值；每个示例在任务里写明核对要点。
2. **标准版本越界**：示例误用高于本章声明标准的特性，会让学习者在 C++11 基线下编译失败——每章任务列明允许的特性范围，验证时用 `-std=c++11`（专题章除外）实际编译。
3. **反例污染**：故意演示未定义行为、悬垂指针、数据竞争的示例若不隔离，学习者会把错误写法当范式——反例文件统一以 `bad_` 前缀命名、单独构建、正文明确标注"反例"，并用 sanitizer 给出检出结果。
4. **前置知识断裂**：后章用到前章没教过的概念——每章任务的"前置"字段逐项列明依赖的前章知识点，产出时核对均已出现。
5. **第三方库版本漂移**：vsomeip、Fast DDS 升级后 API/构建步骤变化——车载专题章节固定写明实测版本与完整构建命令，后续引用不得悄悄换版本。

---

## 每章生产流程（所有章节共用，任务里不再重复）

1. 按六段结构写本章 `README.md` 正文。
2. 写 `examples/CMakeLists.txt` 与任务清单中的示例源码（关键处中文注释）。
3. 在验证环境构建并运行全部示例，逐项核对任务里写明的预期输出/检出结果；不过关就改代码或改正文，直到一致。
4. 写 `exercises.md`（练习由浅入深），参考答案放同章 `solutions/`（代码类答案为可编译源码，文字类答案为 Markdown）。
5. 更新 `LearnLinux/README.md` 进度表（本章状态改为已交付 + 日期）。
6. 将本章全部文件提交到仓库对应路径。
7. 暂停：向学习者汇报本章要点、示例验证结果、练习安排；得到确认后进入下一章。

## 文件结构（第一部分）

```
LearnLinux/
├── part01-foundations/
│   ├── ch01-pointers-memory-1/   {README.md, exercises.md, examples/, solutions/}
│   ├── ch02-pointers-memory-2/
│   ├── ch03-reference-const-funcptr/
│   ├── ch04-templates-1/
│   ├── ch05-templates-2/
│   ├── ch06-raii-smartptr-move/
│   └── project-dynarray/         {README.md（任务书）, starter/, solutions/}
├── part02-linux-basics/          （路线图见下）
├── part03-concurrency/
├── part04-network-ipc/
├── part05-automotive/
└── capstone/
```

---

## 第一部分：地基补强（逐章详细计划）

### Task 1：第 1 章 指针与内存（一）

**Files:**
- Create: `LearnLinux/part01-foundations/ch01-pointers-memory-1/README.md`
- Create: `LearnLinux/part01-foundations/ch01-pointers-memory-1/exercises.md`
- Create: `LearnLinux/part01-foundations/ch01-pointers-memory-1/examples/CMakeLists.txt`
- Create: `.../examples/ex01_address.cpp`、`ex02_deref.cpp`、`ex03_pointer_and_array.cpp`、`ex04_pointer_arith.cpp`

**本章知识点：** 变量与地址、`&` 与 `*`、指针的声明与类型、`sizeof` 指针、指针与数组的关系、数组名退化、指针运算与边界。
**前置：** 学习者已有的 C 基础（变量、数组、函数）。
**示例与核对要点：**
- `ex01_address`：打印 int/double/char 变量的地址与值；核对：地址格式为十六进制、不同变量地址互不相同。
- `ex02_deref`：通过指针修改变量值；核对：输出修改前后两行，值按正文所述变化。
- `ex03_pointer_and_array`：数组元素地址逐个 +sizeof(元素)；核对：相邻元素地址差等于元素大小。
- `ex04_pointer_arith`：指针遍历数组求和；核对：输出总和为正文给定的确定值。
**验证命令：** `cmake -S examples -B build && cmake --build build` 后逐个运行四个可执行文件，与上述要点比对。

### Task 2：第 2 章 指针与内存（二）

**Files:** 同上结构，目录 `ch02-pointers-memory-2/`；示例 `ex01_double_pointer.cpp`、`ex02_heap_stack.cpp`、`ex03_nullptr.cpp`、`bad_01_dangling.cpp`（反例，单独构建）。

**本章知识点：** 多级指针、指针的指针在"改指向"场景中的用途、`void*`、nullptr 与 NULL 的区别（C++11）、栈与堆、`new/delete` 与 `new[]/delete[]` 配对、内存布局（代码段/数据段/BSS/堆/栈）、野指针与悬垂指针的成因。
**前置：** 第 1 章全部。
**示例与核对要点：**
- `ex01_double_pointer`：函数通过二级指针让指针指向新数组；核对：调用后打印新数组内容与正文一致。
- `ex02_heap_stack`：对比栈数组与堆数组的生命周期写法；核对：正常运行、无泄漏提示（可选 valgrind 演示一笔带过）。
- `ex03_nullptr`：nullptr 与重载决议（`f(int)` vs `f(int*)`）；核对：输出命中的是 指针重载。
- `bad_01_dangling`（反例）：释放后继续使用；核对：以 `-fsanitize=address` 构建运行，预期 ASan 报告 heap-use-after-free，正文贴检出摘要并讲清成因与规避（置 nullptr、明确所有权预告第 6 章）。

### Task 3：第 3 章 引用、const 与函数指针

**Files:** 目录 `ch03-reference-const-funcptr/`；示例 `ex01_reference.cpp`、`ex02_const_pointer.cpp`、`ex03_ref_params.cpp`、`ex04_func_pointer.cpp`。

**本章知识点：** 引用的本质与初始化要求、引用 vs 指针的取舍、顶层/底层 const、指针常量与常量指针、引用传参（改实参、避免拷贝、const 引用）、函数指针、回调（比较器）、`using` 类型别名。
**前置：** 第 1～2 章。
**示例与核对要点：**
- `ex01_reference`：别名改值、引用不可改绑；核对：输出值变化与正文一致。
- `ex02_const_pointer`：四种 const 组合各一例，其中错误写法以注释形式给出并标注编译器报错要点；核对：合法部分编译通过。
- `ex03_ref_params`：swap 的指针版与引用版对照；核对：交换结果正确，并打印大对象按值传参 vs const 引用传参的拷贝计数差异（计数值为确定值）。
- `ex04_func_pointer`：函数指针数组实现简易计算器分发 + 回调比较器排序；核对：计算与排序结果为正文给定值。

### Task 4：第 4 章 模板基础（一）

**Files:** 目录 `ch04-templates-1/`；示例 `ex01_func_template.cpp`、`ex02_deduction.cpp`、`ex03_class_template.cpp`、`ex04_overload_with_template.cpp`。

**本章知识点：** 为何需要模板、函数模板写法、类型推导规则（引用推导、数组退化在模板中的表现）、类模板写法与成员函数定义位置、实例化的概念、模板代码为何通常放头文件。
**前置：** 第 1～3 章（尤其指针与引用推导）。
**示例与核对要点：**
- `ex01_func_template`：`maxValue<T>` 对 int/double/字符串的使用；核对：输出各最大值正确。
- `ex02_deduction`：同一模板以值/引用/数组实参调用，打印推导类型名（`typeid` 名称只作对照，正文说明其实现相关）；核对：按正文表格逐行对应。
- `ex03_class_template`：`Pair<T1,T2>` 与 `Box<T>`；核对：存取与打印结果正确。
- `ex04_overload_with_template`：普通函数与模板同名共存的决议顺序；核对：每次调用命中的版本与正文预测一致（输出中标注来源）。

### Task 5：第 5 章 模板基础（二）

**Files:** 目录 `ch05-templates-2/`；示例 `ex01_specialization.cpp`、`ex02_partial_spec.cpp`、`ex03_nontype_param.cpp`、`ex04_variadic_print.cpp`。

**本章知识点：** 全特化、偏特化（指针偏特化）、非类型模板参数、可变参数模板与递归展开（C++11）、模板常见报错的逐段解读法、类型萃取概念预告。
**前置：** 第 4 章。
**示例与核对要点：**
- `ex01_specialization`：通用版与 `const char*` 特化版的类型描述函数；核对：两种调用输出不同分支且与正文一致。
- `ex02_partial_spec`：`Box<T>` 与 `Box<T*>` 偏特化，后者多一个解引用接口；核对：指针版输出解引用结果。
- `ex03_nontype_param`：`FixedArray<T, N>` 编译期定长数组；核对：`static_assert` 校验大小，运行输出元素和为给定值。
- `ex04_variadic_print`：C++11 递归展开实现 `printAll`；核对：多类型参数按序打印。
- 正文含一节"读懂模板报错"：以 `ex01` 的一次故意写错为例，贴真实报错并逐段拆解（报错原文在验证环境实测截取）。

### Task 6：第 6 章 RAII、智能指针与移动语义

**Files:** 目录 `ch06-raii-smartptr-move/`；示例 `ex01_raii_file.cpp`、`ex02_unique_ptr.cpp`、`ex03_shared_weak.cpp`、`ex04_move_semantics.cpp`。

**本章知识点：** RAII 思想与资源清单（内存/文件/锁预告）、`unique_ptr` 与所有权、`shared_ptr` 引用计数、`weak_ptr` 破循环引用、`make_shared`（`make_unique` 为 C++14、随用随讲时标注）、左值/右值、右值引用、移动构造与移动赋值、`std::move`、拷贝消除一瞥。
**前置：** 第 1～5 章。
**示例与核对要点：**
- `ex01_raii_file`：手写文件 RAII 封装类（构造打开、析构关闭、禁用拷贝）；核对：作用域退出后文件可被再次打开写入。
- `ex02_unique_ptr`：所有权转移与数组版；核对：转移后源指针为空（输出 bool 值）。
- `ex03_shared_weak`：先写 shared_ptr 循环引用导致不释放的对照版（输出析构计数为 0），再用 weak_ptr 修正（计数归零、析构发生）；两版输出对照。
- `ex04_move_semantics`：带拷贝/移动计数的大对象，分别触发拷贝与移动；核对：计数输出与正文预测的确定数值一致。

### Task 7：第一部分阶段项目——模板动态数组 DynArray

**Files:**
- Create: `LearnLinux/part01-foundations/project-dynarray/README.md`（任务书：需求、验收标准、提示）
- Create: `.../project-dynarray/starter/dynarray.h`（类骨架：接口齐全、函数体留空并标注 TODO）、`starter/main.cpp`（验收测试，断言序列固定）
- Create: `.../project-dynarray/solutions/dynarray.h`、`solutions/main.cpp`（参考实现，与 starter 的验收测试共用一份断言）

**需求（任务书要点）：** 模板类 `DynArray<T>`：默认构造、按大小构造、拷贝构造/拷贝赋值（深拷贝）、移动构造/移动赋值（C++11）、析构释放、`operator[]`（含 const 版）、`size()/capacity()`、`push_back`（容量不足时翻倍扩容）、`begin()/end()` 支持范围 for。
**验收：** 参考实现与验收测试在 `-std=c++11 -Wall -Wextra -fsanitize=address,undefined` 下构建运行，全部断言通过、无 sanitizer 报告；学习者用同一份 `main.cpp` 验收自己的实现。
**Interfaces:** 本项目产出的 RAII + 容器写法，将在第三部分线程安全容器与毕业项目中复用，类名与方法名以此处为准。

---

## 第二至第六部分：分章路线图

> 每部分开始前，按"每章生产流程"与第一部分同样的粒度（知识点/前置/示例与核对要点/验证命令）补齐该部分逐章任务，补齐稿随该部分第一章一并请学习者过目；章节标题以下列路线图为准，顺序不得擅自变更，增删须经学习者确认。

### Task 8：第二部分 Linux 开发起步（5 章 + 阶段项目）
- ch01 工具链与环境搭建：GCC 编译流程、CMake 工程组织、GDB 调试、按基线对齐开发机环境。
- ch02 文件与系统调用：fd、open/read/write、阻塞与非阻塞。
- ch03 进程（一）：fork/exec、进程映像、僵尸与孤儿、wait 回收。
- ch04 进程（二）与守护进程：进程组/会话、守护进程标准写法、日志去向。
- ch05 信号：signal/sigaction、异步信号安全、SIGCHLD 回收子进程、定时器信号。
- 阶段项目：多进程日志收集工具（子进程写、父进程管，信号回收）。

### Task 9：第三部分 并发与多线程（6 章 + 阶段项目）
- ch01 线程模型：std::thread 与 pthread、参数传递、生命周期与回收。
- ch02 同步原语（一）：互斥锁、lock_guard/unique_lock、读写锁。
- ch03 同步原语（二）：条件变量、生产者消费者、信号量（POSIX 与 C++20 预告）。
- ch04 原子操作与内存模型入门：atomic、计数器实验、无锁队列思想。
- ch05 并发安全与死锁：数据竞争定位（TSan 实测）、死锁成因与规避、反例章。
- ch06 线程池 + C++20 并发专题：线程池实现；专题：jthread、latch/barrier、协程初步。
- 阶段项目：线程安全任务队列 + 日志库（毕业项目复用，接口在此定型）。

### Task 10：第四部分 网络与 IPC（6 章 + 阶段项目）
- ch01 Socket 基础：地址结构、字节序、TCP 客户端/服务器最小闭环。
- ch02 TCP/UDP 深入：粘包与消息边界、UDP 特点与适用场景。
- ch03 I/O 多路复用：select/poll/epoll 三级推进、Reactor 模式。
- ch04 IPC 全景：管道、消息队列、信号量集、Unix 域套接字。
- ch05 共享内存：shm/mmap、同步问题、环形缓冲区设计与实现。
- ch06 序列化基础：结构体对齐与填充、二进制协议设计、Protobuf 概念一瞥。
- 阶段项目：epoll 服务器 + 共享内存数据通道（二者对接）。

### Task 11：C++17 专题（独立专题，置于第四部分之后、车载专题之前；2～3 章）
- 结构化绑定、if constexpr、string_view、optional/variant。
- filesystem、并行算法、类模板参数推导（CTAD）。
> 说明：本专题是对设计文档"C++17 设专题章"的落实，章数在补齐该部分逐章任务时定（2～3 章）。

### Task 12：第五部分 车载专题（7 章）
- ch01 SocketCAN（一）：CAN 帧结构、vcan 搭建、原始套接字收发。
- ch02 SocketCAN（二）：手写信号表解码（位提取、大小端、缩放）、DBC 概念与生成工具路径。
- ch03 SOME/IP 协议：报文结构、服务发现、请求/响应与事件通知。
- ch04 vsomeip 实操：源码构建、routing manager、Offer/Request 与事件订阅的一对服务/客户端。
- ch05 Fast DDS（一）：源码构建、IDL 与类型生成、发布/订阅、自动发现。
- ch06 Fast DDS（二）：QoS 策略体系与实战调法（可靠性、历史、存活、截止期）。
- ch07 Vehicle HAL 模拟：属性模型、独立进程服务（Unix 域套接字）实现 get/set/subscribe、按真实 VHAL 类型建模。
- 期间穿插 C++20 专题（2～3 章，置于 DDS 之后或与 ch07 之后衔接）：concepts、ranges、协程、format 与时间库；具体位置在补齐本部分逐章任务时定。

### Task 13：第六部分 毕业项目——座舱信号网关（3～4 章）
- ch01 需求与架构：信号清单（模拟 DBC 子集）定稿、模块划分、线程模型。
- ch02 实现（一）：CAN 接入与解码模块（vcan，可切换模拟源）。
- ch03 实现（二）：内部线程安全分发（复用第三部分任务队列）+ SOME/IP 发布 + Fast DDS 发布双通道。
- ch04 实现（三）：HAL 风格属性接口层接入、异常处理、联调测试与复盘。

---

## 执行说明

- 执行方式：由作者逐章自产（Native），每章走完"每章生产流程"后暂停等学习者确认——此节奏为学习者明确选定，不采用子代理批量产出。
- 本计划与设计文档一同存放于 `LearnLinux/docs/`；第二至第六部分的逐章补齐稿以附录形式追加到本计划或另立分部分计划文件，存放位置相同。
