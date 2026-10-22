// ex03_lseek.cpp —— 文件偏移：随机读与求大小；以及管道上为什么不行
// 编译基线：C++11
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

static void writeFixture() {
    int fd = open("ex03_data.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    const char* data = "0123456789ABCDEF";   // 固定 16 字节
    write(fd, data, std::strlen(data));
    close(fd);
}

int main() {
    writeFixture();

    int fd = open("ex03_data.txt", O_RDONLY);
    char buf[32] = {0};

    // 定位到偏移 10 再读：跳过前面的 0~9
    lseek(fd, 10, SEEK_SET);
    ssize_t n = read(fd, buf, 6);
    std::printf("from offset 10: %.*s\n", (int)n, buf);        // ABCDEF

    // SEEK_END 返回值就是文件大小；再回退 4 字节读尾
    off_t size = lseek(fd, 0, SEEK_END);
    std::printf("file size = %lld\n", (long long)size);        // 16
    lseek(fd, -4, SEEK_END);
    std::memset(buf, 0, sizeof(buf));
    n = read(fd, buf, 4);
    std::printf("last 4 bytes: %.*s\n", (int)n, buf);           // CDEF
    close(fd);

    // 管道没有"偏移"这个概念：lseek 必然失败，errno = ESPIPE
    int p[2];
    pipe(p);
    off_t r = lseek(p[0], 0, SEEK_SET);
    std::printf("lseek on pipe: ret = %lld, errno = %d (ESPIPE = %d)\n",
                (long long)r, errno, ESPIPE);                  // -1, 29
    close(p[0]);
    close(p[1]);
    return 0;
}
