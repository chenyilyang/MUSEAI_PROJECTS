# 第 1 章 Socket 基础：地址、字节序与 TCP 最小闭环（第四部分 · 网络与 IPC）

## 本章目标

- 会用 `socket` / `bind` / `listen` / `accept` / `connect` 建起 TCP 最小闭环，并能说清每一步在干什么；
- 理解地址结构（`sockaddr_in`）与字节序（主机序 vs 网络序），以及为什么转换函数必须成对出现；
- 掌握 TCP 的"字节流"语义：`send`/`recv` 的返回值是"本次实际传了多少"，收发循环是必备姿势；
- 养成两个工程习惯：回环演示用动态端口（`bind` 端口 0 + `getsockname`），不依赖固定端口；错误处理先写对 `EINTR` 与返回值检查。

## 1. 地址：sockaddr_in 里到底装了什么

一个 IPv4 地址 + 端口，在代码里就是 `sockaddr_in`：

```c
struct sockaddr_in {
    sa_family_t    sin_family;   // AF_INET：IPv4
    in_port_t      sin_port;     // 端口，网络字节序
    struct in_addr sin_addr;     // IPv4 地址，网络字节序
    unsigned char  sin_zero[8];  // 填充，对齐用
};
```

三个注意点：

1. **端口与地址填进去之前要先转网络字节序**：`htons(port)`、`htonl(INADDR_LOOPBACK)`。忘了转，程序在小端机器上会连到完全错误的端口——这是 socket 新手最常见的"程序能编译、运行就是不通"的根源。
2. **`sockaddr` 是通用容器**：`bind`/`connect`/`accept` 的参数类型都是 `sockaddr*`，实际传 `sockaddr_in*` 时要显式转换；IPv6（`sockaddr_in6`）与 Unix 域（`sockaddr_un`）共用同一套函数签名，区别只在地址结构与 `sin_family`——第 4 章讲 Unix 域套接字时，这套代码几乎原样复用。
3. **字符串与二进制地址互转**：`inet_pton(AF_INET, "127.0.0.1", &addr)`（文本→二进制）、`inet_ntop`（反向）。演示里直接用 `htonl(INADDR_LOOPBACK)`，因为回环地址是常量；真实代码里地址来自配置或 DNS，`inet_pton` 才是常态。

## 2. 字节序：为什么要有网络字节序

不同 CPU 存多字节整数的顺序不同：x86/ARM 默认小端（低字节在低地址），网络协议规定大端（"网络字节序"）。两台字节序不同的机器直接传内存，`0x12345678` 到对端会变成 `0x78563412`——不是"乱码"，是**确定的错**，所以必须统一。

四个转换函数（`arpa/inet.h`），名字即语义：

| 函数 | 方向 |
|---|---|
| `htons` / `htonl` | 主机序 → 网络序（s = short 16 位，l = long 32 位） |
| `ntohs` / `ntohl` | 网络序 → 主机序 |

`ex01_byteorder.cpp` 的断言是按"与主机无关"的方式写的：`htonl(0x12345678)` 落到内存里的字节恒为 `12 34 56 78`，`ntohl(htonl(x)) == x` 在任何主机上都成立——转换函数在大小端主机上做相反的事，恰好抵消。本章实测（小端主机）：

```
host byte order: little-endian
network bytes of 0x12345678: 12 34 56 78
round-trip: 0x12345678 -> htonl -> ntohl = 0x12345678
```

## 3. TCP 最小闭环：服务端四步，客户端两步

`ex02_tcp_echo.cpp` 是全章的锚。服务端：

```
socket() → bind() → listen() → accept() → recv() → send() → close()
```

客户端：

```
socket() → connect() → send() → recv() → close()
```

逐条说明：

- **`bind` 的端口填 0**：让内核分配空闲端口，再用 `getsockname` 读回实际值。本章所有演示都这么做——写死 8080 的演示会在端口被占时莫名失败，而动态端口永远不会。服务端把端口经 `std::promise` 交给主线程，保证客户端在 `listen` 就绪后才 `connect`，没有竞态。
- **`listen` 的第二个参数**（backlog）是"已完成连接但未被 accept 的队列上限"，不是"最大连接数"；演示里填 1 够用，生产服务按并发量给。
- **`accept` 返回的是新 fd**：监听 fd 只负责接连接，收发走 accept 返回的 fd；一个监听 fd 可以 accept 出很多个连接 fd——第 3 章的 epoll 就是围着这个结构转的。
- **`SO_REUSEADDR`**：允许 TIME_WAIT 状态的地址被立即重用。演示里不设它也能跑，但反复起停服务时，没有它，下一次 `bind` 会被"地址已被占用"挡住——这是联调时最常见的拦路虎之一。

## 4. 字节流语义：send/recv 的返回值是契约

TCP 不保存消息边界：`send(fd, buf, 1000)` 返回 600，只表示"这一次发出了 600"，剩下的 400 要你自己接着发；`recv` 同理，可能只给你一部分。`ex03_send_recv_all.cpp` 把这个姿势封装成两个函数：

```cpp
bool sendAll(int fd, const char* buf, std::size_t len);  // 发满才返回真
bool recvAll(int fd, char* buf, std::size_t len);        // 收满才返回真
```

循环里有三条纪律：

1. **推进指针、扣减剩余**：每次按实际返回值 `n` 前进 `buf += n; len -= n;`，不是重发同一段；
2. **`EINTR` 不是失败**：`send`/`recv` 被信号打断返回 -1 且 `errno == EINTR` 时，继续循环，不是报错退出；
3. **`recv` 返回 0 是对端关闭**：数据没凑够就遇到 0，说明对端提前收了线——这是"没传完"的信号，不是"本次没数据"。

本章实测：100000 字节模式数据（第 i 字节 = i % 251），服务端循环收满、独立算校验和：

```
received 100000 bytes
checksum match
```

这个"循环收满"的习惯，是下一章粘包/成帧的全部前置知识：**流本身没有边界，边界只能由应用层自己划**。

## 示例代码：构建、运行与验收

```bash
cmake -S examples -B examples/build && cmake --build examples/build
./examples/build/ex01_byteorder
./examples/build/ex02_tcp_echo
./examples/build/ex03_send_recv_all
bash tests/run_tests.sh                  # 全章验收（含 solutions 与 TSan 复核）
```

**验证记录（测试先行）**：`tests/run_tests.sh` 先于全部示例写定，首跑在构建阶段判红（passed=0）。实现后连跑三轮 **7/7 全绿、零警告**（构建开 `-Wall -Wextra -Werror`），`ex02` 另经 TSan 构建复核零报告（端口经值传递、收发双方无共享可变状态，干净是结构保证的）。

**沙箱实情（UDP 一项）**：作者环境禁 UDP——裸 UDP `sendto` 到回环即报 `EPERM`，与代码无关。练习 3 的 UDP 回显代码已完整入库且编译通过，但在本机未能实测；验收脚本先探测 UDP 回环是否可用，不可用时明示跳过（SKIP），不计失败。请在自己的开发机上运行 `solutions/build/ex03_udp_echo` 验证该项，预期输出 `udp echo OK: hello udp`。

## 动手练习

见 `exercises.md`，共 4 题：字节序问答（文字）、一问一答多轮回显（代码）、UDP 版 echo（代码）、流语义与粘包的关系（思考）。代码题参考答案在 `solutions/`。

## 自测清单

- [ ] 能默写服务端四步与客户端两步，并说清 `accept` 返回的新 fd 是干什么的
- [ ] `sin_port` 与 `sin_addr` 填入前为什么要先过 `htons` / `htonl`
- [ ] `bind` 端口填 0 的好处；`getsockname` 在哪一步之后才能读到真实端口
- [ ] `send` 返回 600（请求 1000）时，正确做法是什么；`recv` 返回 0 意味着什么
- [ ] `EINTR` 出现时为什么应该继续循环而不是报错
- [ ] `SO_REUSEADDR` 解决的是哪个阶段的什么问题
- [ ] TCP 与 UDP 在本章演示里的步骤差异（少了哪两步），以及这个差异对应的语义代价

## 本章小结与下一章预告

本章立住了两块地基：地址与字节序（数据怎么表示）、最小闭环与收发循环（数据怎么流动）。下一章直面本章埋下的伏笔：**流没有边界，消息边界必须由应用层划**——粘包的复现、长度前缀与分隔符两种成帧法，以及 UDP 的数据报语义与它的取舍。
