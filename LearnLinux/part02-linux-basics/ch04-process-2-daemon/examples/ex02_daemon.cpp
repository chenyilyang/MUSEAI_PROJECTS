// ex02_daemon.cpp —— 守护进程的标准写法（完整版）
// 步骤：fork 让父退场 → setsid 脱离终端 → 再 fork 确保永远拿不到控制终端 →
//       umask 清零 → chdir 到根目录 → 标准流重定向到 /dev/null →
//       此后一切输出只写日志文件。
// 运行：./ex02_daemon 立刻返回；用 /tmp/learnlinux_daemon.pid 找它的 PID，
//       看 /tmp/learnlinux_daemon.log 的心跳，最后 kill -TERM <pid> 让它收尾。
// 编译基线：C++11
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

static volatile std::sig_atomic_t g_stop = 0;
static void onTerm(int) { g_stop = 1; }   // 处理函数只置标志，重活回主循环（第 5 章细讲）

static void writePidFile() {
    int fd = open("/tmp/learnlinux_daemon.pid", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) {
        char buf[32];
        int n = std::snprintf(buf, sizeof(buf), "%d\n", getpid());
        write(fd, buf, (size_t)n);
        close(fd);
    }
}

int main() {
    // 1) 第一次 fork：父进程退出，子进程被"过继"，且子进程不是组长 → 有资格 setsid
    pid_t pid = fork();
    if (pid < 0) return 1;
    if (pid > 0) return 0;                        // 启动器到此为止

    // 2) 开新会话：脱离原控制终端
    if (setsid() < 0) _exit(1);

    // 3) 第二次 fork：新进程不是会话首进程，今后打开任何终端设备都不会
    //    把它变成自己的控制终端——与终端彻底绝缘
    pid = fork();
    if (pid < 0) _exit(1);
    if (pid > 0) _exit(0);

    // 4) 文件创建权限不再受启动者 umask 影响；工作目录改到根目录，
    //    免得钉住某个目录让它无法卸载
    umask(0);
    chdir("/");

    // 5) 标准流全部改指 /dev/null：读不到终端输入，也不会往任何终端乱写
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);

    // 6) 从这里起只能用日志文件说话
    signal(SIGTERM, onTerm);
    writePidFile();
    int log = open("/tmp/learnlinux_daemon.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log < 0) _exit(1);

    int beats = 0;
    while (!g_stop) {
        char buf[80];
        int n = std::snprintf(buf, sizeof(buf), "heartbeat %d from daemon pid %d\n",
                              ++beats, getpid());
        write(log, buf, (size_t)n);
        sleep(1);       // 被 SIGTERM 打断时会提前醒来，循环条件随即生效
    }
    const char* bye = "daemon exiting on SIGTERM\n";
    write(log, bye, std::strlen(bye));
    close(log);
    return 0;
}
