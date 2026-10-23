// log_collector.cpp —— 阶段项目骨架：能跑，但离规格还差四件事（见 TODO）
// 已备好：参数解析、管道、worker 产出、父进程读管道盖时间戳落盘。
// 未完成：信号接管、子进程回收、SIGTERM 优雅关闭、收尾输出。
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <string>
#include <unistd.h>

// "[YYYY-MM-DDTHH:MM:SS] " 形式的当前时间前缀
static std::string nowPrefix() {
    std::time_t t = std::time(NULL);
    struct tm tmv;
    localtime_r(&t, &tmv);
    char buf[32];
    std::strftime(buf, sizeof(buf), "[%Y-%m-%dT%H:%M:%S] ", &tmv);
    return std::string(buf);
}

// 把 buf 完整写出（处理部分写与 EINTR）
static void writeAll(int fd, const char* p, size_t len) {
    while (len > 0) {
        ssize_t w = write(fd, p, len);
        if (w < 0) {
            if (errno == EINTR) continue;
            return;
        }
        p += w;
        len -= (size_t)w;
    }
}

// worker：每行拼好后一次 write 写完（防多写者行穿插），行间小睡，_exit(编号)
static void workerMain(int idx, int lines, int writeFd) {
    char buf[64];
    for (int seq = 1; seq <= lines; ++seq) {
        int n = std::snprintf(buf, sizeof(buf), "worker-%d msg %d\n", idx, seq);
        writeAll(writeFd, buf, (size_t)n);
        usleep(2000);
    }
    close(writeFd);
    _exit(idx);
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::fprintf(stderr, "usage: %s <logfile> <workers> <lines>\n", argv[0]);
        return 2;
    }
    const char* logPath = argv[1];
    int workers = std::atoi(argv[2]);
    int lines = std::atoi(argv[3]);
    if (workers <= 0 || lines <= 0) {
        std::fprintf(stderr, "workers and lines must be positive\n");
        return 2;
    }

    int logFd = open(logPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (logFd < 0) { std::perror("open logfile"); return 1; }

    int pipeFd[2];
    if (pipe(pipeFd) < 0) { std::perror("pipe"); return 1; }

    // TODO(1)：在 fork 之前用 sigaction 安装 SIGCHLD 与 SIGTERM 的处理函数，
    //           处理函数里只许置 volatile sig_atomic_t 标志。

    for (int i = 0; i < workers; ++i) {
        pid_t pid = fork();
        if (pid < 0) { std::perror("fork"); return 1; }
        if (pid == 0) {
            close(pipeFd[0]);            // 子进程只用写端
            workerMain(i, lines, pipeFd[1]);
        }
        // TODO(3) 的一半：把每个 worker 的 pid 记下来，后面回收与转发信号都要用
    }
    close(pipeFd[1]);                    // 父进程关掉自己的写端副本（不关，EOF 永不来）

    // TODO(2)：把读端改为非阻塞，主循环里同时做三件事——读数据、（凭 SIGCHLD
    //           标志）排干回收、（凭 SIGTERM 标志）启动优雅关闭。
    //           当前骨架只会阻塞读到 EOF 为止：
    std::string pending;
    char chunk[4096];
    for (;;) {
        ssize_t r = read(pipeFd[0], chunk, sizeof(chunk));
        if (r == 0) break;               // EOF：所有写端都已关闭
        if (r < 0) {
            if (errno == EINTR) continue;
            std::perror("read");
            break;
        }
        pending.append(chunk, (size_t)r);
        std::string::size_type pos;
        while ((pos = pending.find('\n')) != std::string::npos) {
            std::string line = pending.substr(0, pos);
            pending.erase(0, pos + 1);
            std::string outLine = nowPrefix() + line + "\n";
            writeAll(logFd, outLine.c_str(), outLine.size());
        }
    }
    close(pipeFd[0]);
    close(logFd);

    // TODO(3)：回收全部 worker（WNOHANG 排干循环），汇总退出码，用 ECHILD 自证
    //           无僵尸，并按规格打印三行收尾输出。
    // TODO(4)：SIGTERM 到达时转发给所有存活 worker，读完管道存量，写下
    //           "collector shutdown" 收尾行，再以退出码 0 结束。
    return 0;
}
