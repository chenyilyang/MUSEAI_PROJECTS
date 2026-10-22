// 练习 3 参考答案：fork + execl 运行 /bin/ls，父进程解码退出状态
#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>

static int runLs(const char* dir) {
    pid_t pid = fork();
    if (pid < 0) { std::perror("fork"); return -1; }
    if (pid == 0) {
        execl("/bin/ls", "ls", dir, (char*)NULL);
        std::perror("execl");        // 走到这里说明 exec 没成功
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) return WEXITSTATUS(status);
    return -1;                       // 被信号终止等异常情况
}

int main() {
    int code1 = runLs("/tmp");
    std::printf("ls /tmp -> exit code %d\n", code1);                       // 0
    int code2 = runLs("/definitely/not/a/dir");
    std::printf("ls /definitely/not/a/dir -> exit code %d\n", code2);     // 2（ls 的报错码）
    return 0;
}
