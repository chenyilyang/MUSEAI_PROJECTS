# 第 2 章 练习（第二部分）

共 5 题。先自己动手，再到 `solutions/` 对照（文字题答案在 `solutions/answers.md`，代码题答案是可编译运行的源码）。

## 练习 1（预测 fd 的分配）

```cpp
int a = open("f1", O_RDONLY);      // 设各步都成功
int b = open("f2", O_RDONLY);
close(a);
int c = open("f3", O_RDONLY);
close(1);                          // 把标准输出关了（真实代码别乱学，这里只看分配规则）
int d = open("f4", O_WRONLY);
```

写出 a、b、c、d 各自的数值，并用一条规则解释全部四个答案。

## 练习 2（找出读 errno 的错误姿势）

下面三个片段都有毛病，逐个指出错在哪、正确写法是什么：

```cpp
// 片段 A
errno = 0;
int fd = open(path, O_RDONLY);
if (errno != 0) { /* 判定失败并处理 */ }

// 片段 B
int fd = open(path, O_RDONLY);
if (fd < 0) {
    log_to_file("open failed, retrying");   // 先记一行日志
    report_error(errno);                    // 再取 errno 上报
}

// 片段 C
ssize_t n = read(fd, buf, sizeof(buf));
if (n <= 0) { /* 一律按出错处理，打印 errno */ }
```

## 练习 3（写代码：文件拷贝）

用 open/read/write 实现 `copyFile(src, dst)`：把源文件完整复制到目标文件。要求：读、写都用循环顶住部分读写；任何一步失败要 perror 并返回失败；含 0 字节的二进制内容也要原样复制。用一个 5000 字节的自造二进制文件自测（提示：拷完用 `cmp` 命令对比两文件）。

## 练习 4（写代码：读文件尾）

用 lseek + read 实现 `printTail(path, n)`：打印文件最后 n 个字节。要求：文件比 n 还短时打印全文；不许把整个文件读进内存。拿正文 ex03 的 16 字节文件和一段更长的文本各测一次。

## 练习 5（思考题）

(a) 为什么说"read 返回 0"不等于出错？它在普通文件和管道上各意味着什么？
(b) 一次阻塞 read 和一次非阻塞 read 都返回了 -1，怎么区分"真出错"和"现在没数据"？如果把非阻塞读的 EAGAIN 当成致命错误退出，程序会表现出什么症状？
