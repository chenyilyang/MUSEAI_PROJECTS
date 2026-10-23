# 第 1 章 工具链与环境搭建（第二部分 · Linux 开发起步）

## 本章目标

学完这一章，你应该能做到：

- 说清一次构建的四个阶段各自做什么，并能亲手把它们一步步跑出来；
- 独立制作静态库与动态库、给程序链接它们，并能用 `ldd` 查清一个程序运行时到底依赖哪些库、缺库时怎么救；
- 用 CMake 组织一个多文件工程；
- 用 GDB 完成一次真正的定位：下断点、看变量、顺着数值反推病根，而不是靠加打印猜。

**前置知识**：第一部分全部（你已经用过 CMake 与 g++ 构建六章的示例；本章把"会用"升级为"懂原理"）。本章命令在 Ubuntu 24.04 / GCC 13 上实测；动手前请先按文末的清单对齐你的开发机环境。

**本章文件**：示例在 `examples/`（四个子目录），练习在 `exercises.md`，参考答案在 `solutions/`。

---

## 0. 先对齐环境（在你的开发机上执行）

```bash
g++ --version      # 期望：GCC 13.x
cmake --version    # 期望：3.28 附近（小版本差异不影响本教程）
gdb --version      # 期望：GNU gdb 13 或更高
cat /etc/os-release | head -2   # Ubuntu 24.04 LTS
```

版本对不上先补齐（`sudo apt install build-essential gdb cmake`）。本教程后面所有章节的"实测输出"都是在这套基线下取得的；环境一致，输出才可对照。

## 1. 编译的四个阶段：一次构建到底发生了什么

你敲下的 `g++ hello.cpp -o hello` 其实串行跑了四个阶段。把 `examples/ex01_build_stages/hello.cpp` 拆开看：

```bash
g++ -std=c++11 -E hello.cpp -o hello.i   # ① 预处理：展开 #include 与宏
g++ -std=c++11 -S hello.cpp -o hello.s   # ② 编译：生成汇编
g++ -std=c++11 -c hello.cpp -o hello.o   # ③ 汇编：生成目标文件（二进制，但还不能跑）
g++ hello.o -o hello                     # ④ 链接：拼上库代码，产出可执行文件
./hello                                  # hello, toolchain
```

几个实测观察，值得记住：

- **预处理产物膨胀惊人**：8 行的 hello.cpp，`hello.i` 有 1056 行——`#include <cstdio>` 把一整棵头文件树都贴了进来。这就是编译慢的第一个来源，也是第一部分"模板为什么放头文件"在工具层面的回响。
- **目标文件里有"欠账"**。用 `nm hello.o` 看符号表：

  ```
  0000000000000000 T main
                   U puts
  ```

  `T` 表示本文件**定义**了 main；`U` 表示 printf（被编译器优化成了 puts）是**未定义符号**——本文件只留了个欠条，等链接阶段去 libc 里兑现。链接的本质，就是把所有 .o 的欠条与定义对上账。对不上的那一刻，就是老朋友 `undefined reference`（第 4 章模板放错文件、练习 1 里都会再见到它）。
- 最终产物再 `nm hello`，main 有了真实地址，欠条已清。

日常记牢的分工：**-c 只编译不链接**（产出 .o）；链接是独立的一步；报错信息里出现 `undefined reference` 或 `cannot find -lXXX`，那是链接阶段在喊话，不要回头去改语法。

## 2. 静态库与动态库：同一份代码，两种交付

`examples/ex02_libraries/` 里的小库 calc 有两个函数。主程序只认 `calc.h`——**链接哪种库，主程序源码一个字都不用改**，这是先要建立的直觉。

**静态库（.a）**：本质是若干 .o 的打包文件，链接时把用到的代码**复制进**可执行文件。

```bash
g++ -std=c++11 -Wall -Wextra -c calc.cpp -o calc.o
ar rcs libcalc.a calc.o                          # ar 打包成静态库
g++ -std=c++11 -Wall -Wextra main.cpp -L. -lcalc -o main_static
./main_static                                    # 输出 7 / 12
ldd main_static | grep calc                      # 无输出：calc 已经在文件内部了
```

**动态库（.so）**：链接时只记一笔"运行时需要 libcalc.so"，代码**不进**可执行文件，程序启动时由动态链接器去找。

```bash
g++ -std=c++11 -Wall -Wextra -fPIC -c calc.cpp -o calc_pic.o   # -fPIC：位置无关代码，动态库必备
g++ -shared calc_pic.o -o libcalc.so
g++ -std=c++11 -Wall -Wextra main.cpp -L. -lcalc -o main_shared
./main_shared
# → error while loading shared libraries: libcalc.so: cannot open shared object file
LD_LIBRARY_PATH=. ./main_shared                  # 告诉链接器去当前目录找，这才跑起来
LD_LIBRARY_PATH=. ldd main_shared | grep calc
# → libcalc.so => ./libcalc.so (...)
```

这段实测把动态库的三个关键点一次演完：`-fPIC`/`-shared` 是制作侧的标配；`-L` 只解决**链接期**找得到库，**运行期**是另一套搜索规则（默认目录 + `LD_LIBRARY_PATH` + 可执行文件内的 rpath）；找不到库时程序在 main 之前就被链接器拦下——这个报错请记牢脸熟，线上排查"程序起不来"时它是头号常客。选型层面的取舍（体积、升级、启动确定性）留给练习 5 自己推演。

## 3. 多文件工程与 CMake：把手工命令固化下来

文件一多，手工命令就不可维护了：改一个文件要重编哪些、链接顺序、选项是否一致，全靠人记。`examples/ex03_cmake_project/` 把 greet、calc、main 三个模块交给 CMake：

```cmake
add_executable(demo main.cpp greet.cpp calc.cpp)
```

就这一句的实质：三个源文件各自走四阶段、在链接步拼成一个可执行文件，与第 1 节的手工流程完全等价，只是改由构建系统记住依赖、只重编变动过的文件。构建命令还是你熟悉的那两行：

```bash
cmake -S . -B build && cmake --build build
./build/demo     # hello, cockpit / add(3, 4) = 7 / mul(3, 4) = 12
```

输出与 ex02 的手工构建逐行一致——工具换了，原理没换。后面章节的工程会继续长大（多目录、链接第三方库），CMakeLists 的写法随用随扩，本章先把"它就是四阶段的自动记账员"这件事看透。

## 4. GDB 入门：用调试器代替猜

`examples/ex04_gdb/buggy_sum.cpp` 求 1..10 之和，运行结果 56，正确值 55。差 1——是循环多跑了一次，还是初值不对？不猜，用 GDB 问变量本人。先带调试信息构建（注意 `-g -O0`：优化会打乱代码与源码的对应关系，调试构建的标配是关优化）：

```bash
g++ -std=c++11 -g -O0 buggy_sum.cpp -o buggy_sum
gdb ./buggy_sum
```

会话实录（关键步）：

```
(gdb) break buggy_sum.cpp:10        在 printf 一行下断点
(gdb) run                           程序停在 printf 之前
(gdb) print sum                     $1 = 56   ← 总和确实错了，正好多 1
(gdb) break buggy_sum.cpp:8         改在 sum += i 一行下断点
(gdb) ignore 1 9                    跳过前 9 次命中，停在第 10 次
(gdb) run                           重新运行，停在最后一次累加之前
(gdb) print i                       $2 = 10   ← 循环恰好跑 10 次，次数没问题
(gdb) print sum                     $3 = 46   ← 但此时真值应是 45（1+…+9）：已经多 1
(gdb) run                           再重跑，这次停在第一次命中
(gdb) print i                       $4 = 1
(gdb) print sum                     $5 = 1    ← 还没加任何数，sum 已经是 1：初值就是病根
(gdb) quit
```

读法总结：**先在结果处确认"错成什么样"，再往上游逐段 print 中间量，把出错区间二分缩小**。本题先用第 10 次命中（i=10、sum=46）排除了"循环多跑一次"，再用第一次命中时 sum 已是 1 直接锁定第 6 行的初值。`fixed_sum.cpp` 是修正版，输出 55。这套动作——断点、run、print、按数值推理——就是后面所有章节调试的底盘；单步 `next`/`step`、查看调用栈 `backtrace` 随用随查即可，不必一次背全命令表。

---

## 示例代码：构建与运行

```bash
cd examples
cmake -S . -B build && cmake --build build   # 构建 hello 与 ex04 的两个程序
./build/hello                                # ex01 的最终产物形态
# ex02、ex03 的构建命令即正文第 2、3 节所列（手工构建本身就是教学内容）
```

核对要点：

- `ex01`：四阶段产物齐全；`nm hello.o` 中 main 为 T、puts 为 U；`./hello` 输出固定问候语。
- `ex02`：静态版可直接运行且 ldd 无 calc；动态版裸跑报 `cannot open shared object file`，带 `LD_LIBRARY_PATH` 后正常，ldd 能定位到 ./libcalc.so。
- `ex03`：CMake 构建的 demo 输出与手工构建一致。
- `ex04`：buggy 输出 56、fixed 输出 55；GDB 会话中 print 结果与正文一致（printf 处 sum=56；第 10 次命中 i=10、sum=46；第一次命中时 sum 已为 1）。

> 验证记录：以上命令与输出由作者在 Ubuntu 24.04 / GCC 13.3 / CMake 3.28 / GDB 15.1 环境下实测（含 GDB 批处理会话逐项核对），日期 2026-10-22。

## 动手练习

见本章 `exercises.md`（5 道题；参考答案在 `solutions/`）。

## 自测清单

- [ ] 我能不看稿把四阶段命令写全，并解释 nm 里 T 与 U 的含义
- [ ] 我能独立制作 .a 与 .so 并各链接一份程序，说清 -fPIC 的用途
- [ ] 程序报 `cannot open shared object file` 时，我能说出搜索规则并给出至少两种解法，并分清 -L 只管链接期
- [ ] 我能用 CMake 把一个多文件工程组织起来，并说清它与手工命令的等价关系
- [ ] 我会用 GDB 下断点、print 中间量，并用"二分缩小出错区间"的方法定位一个逻辑 bug

## 本章小结与下一章预告

工具握熟了：构建是四阶段、库有两种交付形态、调试靠断点和数值推理。**下一章（文件与系统调用）**正式进入操作系统本体：文件描述符、open/read/write 的返回值约定，以及每个系统程序员都要交学费的 errno——你的 C++ 程序将第一次直接和内核对话。

## 验收脚本补注（2026-10-23）

本章已按第 5 章起施行的测试先行标准补齐验收脚本 `tests/run_tests.sh`（断言取自上文核对要点，在临时目录重演四组实验：四阶段与 nm 符号、静态/动态库、CMake 与手工构建对照、buggy/fixed 输出与 GDB 批处理三查）。补齐时实测：现有示例全数通过；并做变异抽查——将 ex04 修正版的初值 `int sum = 0` 改回 1（正是本章要定位的那个 bug 本身）——套件如期判红，证明断言有区分力。本章当年的产出方式仍为"实测核对"，此处补的是可复跑的验收环节，不改写历史。
