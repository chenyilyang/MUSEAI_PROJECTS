// 练习 4 参考答案：两个子进程，不同退出码，父进程按 PID 逐个回收
#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>

static pid_t spawnChild(int napSecs, int exitCode) {
    pid_t pid = fork();
    if (pid == 0) {
        sleep((unsigned)napSecs);
        _exit(exitCode);
    }
    return pid;
}

int main() {
    pid_t a = spawnChild(2, 3);   // 慢的那个，退出码 3
    pid_t b = spawnChild(1, 7);   // 快的那个，退出码 7

    // 按指定 PID 回收：谁先结束不重要，报告顺序由我决定
    int status = 0;
    waitpid(a, &status, 0);
    std::printf("child A (%d) exit code = %d\n", a, WEXITSTATUS(status));   // 3
    waitpid(b, &status, 0);
    std::printf("child B (%d) exit code = %d\n", b, WEXITSTATUS(status));   // 7
    return 0;
}
