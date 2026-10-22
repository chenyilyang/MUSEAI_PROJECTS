// 练习 3 参考答案：用系统调用实现文件拷贝
// 要点：read/write 都可能只搬一部分——必须循环；read 返回 0 才是读完。
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

static int copyFile(const char* src, const char* dst) {
    int in = open(src, O_RDONLY);
    if (in < 0) { std::perror("open src"); return -1; }
    int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out < 0) { std::perror("open dst"); close(in); return -1; }

    char buf[4096];
    for (;;) {
        ssize_t n = read(in, buf, sizeof(buf));
        if (n < 0) { std::perror("read"); close(in); close(out); return -1; }
        if (n == 0) break;                       // 读到文件尾，收工
        ssize_t off = 0;                         // 写也要循环：一口可能吞不下全部
        while (off < n) {
            ssize_t w = write(out, buf + off, (size_t)(n - off));
            if (w < 0) { std::perror("write"); close(in); close(out); return -1; }
            off += w;
        }
    }
    close(in);
    close(out);
    return 0;
}

int main() {
    // 先造一个源文件（二进制内容，含 0 字节，验证拷贝不是按字符串来的）
    int fd = open("copy_src.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    char data[5000];
    for (int i = 0; i < 5000; ++i) data[i] = (char)(i % 251);
    write(fd, data, sizeof(data));
    close(fd);

    if (copyFile("copy_src.bin", "copy_dst.bin") != 0) return 1;

    // 回读校验：大小与内容逐字节核对（也可以在 shell 里用 cmp 验证）
    int a = open("copy_src.bin", O_RDONLY);
    int b = open("copy_dst.bin", O_RDONLY);
    char ba[5000], bb[5000];
    ssize_t na = read(a, ba, sizeof(ba));
    ssize_t nb = read(b, bb, sizeof(bb));
    bool same = (na == nb);
    for (ssize_t i = 0; same && i < na; ++i) same = (ba[i] == bb[i]);
    std::printf("copied %zd bytes, identical = %s\n", nb, same ? "yes" : "NO");
    close(a);
    close(b);
    return same ? 0 : 1;
}
