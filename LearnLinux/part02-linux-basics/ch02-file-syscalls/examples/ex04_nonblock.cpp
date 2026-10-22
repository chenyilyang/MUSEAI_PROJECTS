// ex04_nonblock.cpp —— 非阻塞读：EAGAIN 不是失败，是"现在没有，待会儿再来"
// 编译基线：C++11
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int p[2];
    pipe(p);

    // 给读端加上 O_NONBLOCK：先取现有标志，再或上新标志回写
    int flags = fcntl(p[0], F_GETFL);
    fcntl(p[0], F_SETFL, flags | O_NONBLOCK);

    // 空管道 + 非阻塞：read 立即返回 -1，errno=EAGAIN——程序一点不耽误
    char buf[16] = {0};
    ssize_t n = read(p[0], buf, sizeof(buf));
    std::printf("empty nonblock read: n = %zd, errno = %d (EAGAIN = %d)\n",
                n, errno, EAGAIN);                              // -1, 11

    // 写入数据后，再读就成功（非阻塞只改变"没数据时"的行为）
    write(p[1], "ping", 4);
    n = read(p[0], buf, sizeof(buf));
    std::printf("after write: n = %zd, data = %.*s\n", n, (int)n, buf);   // 4, ping

    close(p[0]);
    close(p[1]);
    return 0;
}
