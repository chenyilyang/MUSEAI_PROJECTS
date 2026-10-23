// 练习 4 参考答案：用环境变量决定日志目录
// 读 LEARNLINUX_LOG_DIR，未设置则回退 /tmp；子进程验证环境继承。
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

static const char* logDir() {
    const char* d = std::getenv("LEARNLINUX_LOG_DIR");
    return (d != NULL && d[0] != '\0') ? d : "/tmp";   // 空串也视为未设置
}

int main() {
    char path[256];
    std::snprintf(path, sizeof(path), "%s/app.log", logDir());
    std::printf("parent: log path = %s\n", path);
    std::fflush(stdout);   // fork 前冲刷（第 3 章的纪律）

    pid_t pid = fork();
    if (pid == 0) {
        char cpath[256];
        std::snprintf(cpath, sizeof(cpath), "%s/app.log", logDir());
        std::printf("child : log path = %s (inherited env)\n", cpath);
        std::fflush(stdout);   // _exit 不冲刷缓冲，手动冲
        _exit(0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return 0;
}
