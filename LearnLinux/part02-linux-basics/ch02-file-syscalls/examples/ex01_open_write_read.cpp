// ex01_open_write_read.cpp —— 第一次系统调用闭环：open/write/read/close
// 编译基线：C++11；系统头文件 <fcntl.h>/<unistd.h>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

int main() {
    // O_CREAT 不存在则创建；O_TRUNC 已存在则清空；0644 是新建文件的权限位
    int fd = open("ex01_data.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) { std::perror("open for write"); return 1; }
    std::printf("opened fd = %d\n", fd);          // 3：0/1/2 已被标准流占用

    const char* msg = "hello, syscall\n";
    ssize_t written = write(fd, msg, std::strlen(msg));
    std::printf("wrote %zd bytes\n", written);    // 15
    close(fd);

    int fd2 = open("ex01_data.txt", O_RDONLY);
    if (fd2 < 0) { std::perror("open for read"); return 1; }
    std::printf("reopened fd = %d\n", fd2);       // 又是 3：fd 总是取最小可用号

    char buf[64] = {0};
    ssize_t got = read(fd2, buf, sizeof(buf) - 1);
    std::printf("read %zd bytes: %s", got, buf);  // 15 bytes: hello, syscall
    close(fd2);
    return 0;
}
