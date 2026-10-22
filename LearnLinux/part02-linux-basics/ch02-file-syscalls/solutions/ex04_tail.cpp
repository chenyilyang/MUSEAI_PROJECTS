// 练习 4 参考答案：打印文件最后 N 个字节（lseek + read）
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

static void printTail(const char* path, off_t n) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) { std::perror("open"); return; }
    off_t size = lseek(fd, 0, SEEK_END);
    off_t start = (size > n) ? (size - n) : 0;    // 文件比 N 还短就从头读
    lseek(fd, start, SEEK_SET);
    char buf[256];
    ssize_t got = read(fd, buf, sizeof(buf) - 1);
    if (got > 0) std::printf("tail %lld of %s: %.*s\n",
                             (long long)(size - start), path, (int)got, buf);
    close(fd);
}

int main() {
    int fd = open("tail_fixture.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    const char* text = "CAN frame: 0x123 data ready\n";
    write(fd, text, std::strlen(text));
    close(fd);

    printTail("tail_fixture.txt", 10);    // 最后 10 字节：data ready + 换行
    printTail("tail_fixture.txt", 100);   // N 超过文件长度：输出全文
    return 0;
}
