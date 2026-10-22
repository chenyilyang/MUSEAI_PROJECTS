// ex02_errno.cpp —— errno 的正确读法
// 铁律：先看返回值是否表示失败，再立刻读 errno；成功调用不会清零 errno。
// 编译基线：C++11
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd = open("/nonexistent_dir/nope.txt", O_RDONLY);
    if (fd < 0) {
        int saved = errno;   // 先存下来！后续任何调用（包括 printf）都可能改写 errno
        std::printf("open failed, errno = %d (ENOENT = %d)\n", saved, ENOENT);
        std::printf("strerror: %s\n", std::strerror(saved));
        errno = saved;       // perror 读的是当前 errno，先恢复再给它看
        std::perror("perror says");
    }

    // 成功调用不会把 errno 清零——而且任何中间调用都可能把它改写成别的值。
    // 所以失败的那一刻不立刻存下来，事后读到的必然不是案发现场。
    int ok = open("/dev/null", O_RDONLY);
    if (ok >= 0) {
        int after = errno;   // 紧贴着成功调用立刻读
        std::printf("successful open, fd = %d, errno now = %d (no longer %d: "
                    "success does not reset it, other calls churn it)\n",
                    ok, after, ENOENT);
        close(ok);
    }
    return 0;
}
