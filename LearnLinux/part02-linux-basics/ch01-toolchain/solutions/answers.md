# 第 1 章 练习参考答案（第二部分）

## 练习 1

```bash
g++ -std=c++11 -Wall -Wextra -g -c main.cpp -o main.o
g++ -std=c++11 -Wall -Wextra -g -c net.cpp  -o net.o
g++ -std=c++11 -Wall -Wextra -g -c log.cpp  -o log.o
g++ main.o net.o log.o -o app
```

前三步（编译）各自独立，报错多为语法/声明类；**最后一步（链接）**最可能爆 `undefined reference`：它意味着某个 .o 引用了符号（函数/变量），但所有 .o 与库里都没有它的定义——常见原因是漏链了源文件或库、或只有声明没有实现（回想第一部分第 4 章模板放错文件的那一种链接错误，是同一类问题）。

## 练习 2

（a）`libcalc.so => /opt/app/lib/...` 是本工程自产库（路径在应用目录下）；`libstdc++.so.6` 是系统 C++ 运行库。

（b）跑不了。`not found` 表示动态链接器找不到 libmissing.so，程序启动时直接报 `error while loading shared libraries: libmissing.so: cannot open shared object file` 并退出，main 都进不去（正文 ex02 演示过同款报错）。

（c）两条思路：其一，**运行时找**——把库所在目录加进 `LD_LIBRARY_PATH`，或写进 `/etc/ld.so.conf` 后跑 `ldconfig`；其二，**构建时钉死**——链接时用 `-Wl,-rpath,<目录>` 把搜索路径写进可执行文件（部署到固定目录的产品常用这招）。注意区分：`-L` 只管链接期找库，管不了运行期。

## 练习 3

对照命令序列（在 `ex02_libraries` 目录下）：

```bash
# 动态版
g++ -std=c++11 -Wall -Wextra -fPIC -c calc.cpp -o calc_pic.o
g++ -shared calc_pic.o -o libcalc.so
g++ -std=c++11 -Wall -Wextra main.cpp -L. -lcalc -o main_shared
./main_shared                       # 报错：cannot open shared object file
LD_LIBRARY_PATH=. ./main_shared     # 正常输出 7 / 12
LD_LIBRARY_PATH=. ldd main_shared | grep calc   # libcalc.so => ./libcalc.so

# 静态版
g++ -std=c++11 -Wall -Wextra -c calc.cpp -o calc.o
ar rcs libcalc.a calc.o
g++ -std=c++11 -Wall -Wextra main.cpp -L. -lcalc -o main_static
./main_static                       # 直接可运行
ldd main_static | grep calc         # 无输出：库代码已并入可执行文件
```

注意链接动态版时 -lcalc 优先挑到 .so；若同目录同时有 .a 与 .so，想强制静态可在链接时对该库写全路径或加 `-Wl,-Bstatic` 等参数（了解即可）。

## 练习 4

定位过程（修正版见 `ex04_fixed_avg.cpp`，已编译验证输出 1.500000）：

- 在 `std::printf` 的上一行（`int avg = sum / n;` 之后、或直接断在 printf 一行）下断点，`print sum` 与 `print n`：得到 sum=3、n=2——**原料全对**。由此断定循环没问题，病根只可能在 avg 的计算这一步。
- 再 `print sum / n`：GDB 里同样给出 1——两个 int 相除执行整数除法，3/2 砍掉小数得 1。病根确认：**类型**，不是逻辑。
- 改法：声明 avg 为 double 并在相除前把 sum 转成 double（`static_cast<double>(sum) / n`）。顺带记住：这类"结果差一点、逻辑看着全对"的 bug，先怀疑整数除法与溢出，用 GDB print 中间量是最快的定性手段。

## 练习 5

参考论述（要点齐全即可）：

- **体积与内存**：静态库把代码复制进每个可执行文件，多进程各带一份，总体积与内存占用偏大；动态库全系统一份，多进程共享代码页，省内存。
- **升级**：动态库可只替换 .so 文件完成中间件升级（接口不变时），回滚也方便；静态库必须重新链接、重刷整个可执行包，车规场景下整包升级的验证成本更高。
- **启动与确定性**：静态链接无运行期解析，启动更直接、路径依赖更少，行为确定性好排查；动态库要经历链接器搜索与重定位，环境变量或目录变动都可能影响启动（本章的 not found 就是例子）——但这份灵活也可被 rpath 等手段收敛。
- **倾向**：底层基础库与中间件选动态库（利于独立升级与内存共享），对启动确定性要求极高的早期启动组件或小型工具可选静态。关键不在选边，而在交付时把 ldd 结果作为发布检查项——本章的命令就是为此准备的。
