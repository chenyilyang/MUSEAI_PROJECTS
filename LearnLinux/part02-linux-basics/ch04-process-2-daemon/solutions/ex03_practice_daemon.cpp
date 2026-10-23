// 练习 3 参考答案：带 pid 文件与 SIGTERM 收尾的守护进程
// 与正文 ex02 的差别：心跳间隔 2 秒，日志与 pid 文件换到练习自己的路径。
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static volatile std::sig_atomic_t g_stop = 0;
static void onTerm(int) { g_stop = 1; }

static void daemonize() {
    pid_t pid = fork();
    if (pid < 0) _exit(1);
    if (pid > 0) _exit(0);              // 注意：在"变成守护进程"的程序里父进程用 _exit 也可，
                                        // 本函数由 main 首行调用，父进程无其他收尾
    if (setsid() < 0) _exit(1);
    pid = fork();
    if (pid < 0) _exit(1);
    if (pid > 0) _exit(0);
    umask(0);
    chdir("/");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
}

int main() {
    daemonize();

    int pf = open("/tmp/ex03_practice_daemon.pid", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (pf >= 0) {
        char buf[32];
        int n = std::snprintf(buf, sizeof(buf), "%d\n", getpid());
        write(pf, buf, (size_t)n);
        close(pf);
    }

    signal(SIGTERM, onTerm);
    int log = open("/tmp/ex03_practice_daemon.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log < 0) return 1;
    int beats = 0;
    while (!g_stop) {
        char buf[80];
        int n = std::snprintf(buf, sizeof(buf), "practice heartbeat %d, pid %d\n",
                              ++beats, getpid());
        write(log, buf, (size_t)n);
        // 演示节奏：睡 2 秒（也可拆成两次 1 秒以便更快响应信号）
        sleep(2);
    }
    const char* bye = "practice daemon bye\n";
    write(log, bye, std::strlen(bye));
    close(log);
    return 0;
}
