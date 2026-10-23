// log_collector.cpp —— 阶段项目参考实现
// 总装：pipe（第 2 章）+ fork/wait（第 3 章）+ sigaction 标志位架构（第 5 章）。
// 铁律：信号处理函数只置标志；读数据、回收、关闭动作全在主循环里完成。
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

static volatile std::sig_atomic_t g_childEvent = 0;   // 有子进程状态变化了
static volatile std::sig_atomic_t g_stop = 0;         // 收到关闭请求
static void onChld(int) { g_childEvent = 1; }
static void onStop(int) { g_stop = 1; }

static std::string nowPrefix() {
    std::time_t t = std::time(NULL);
    struct tm tmv;
    localtime_r(&t, &tmv);
    char buf[32];
    std::strftime(buf, sizeof(buf), "[%Y-%m-%dT%H:%M:%S] ", &tmv);
    return std::string(buf);
}

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

static void workerMain(int idx, int lines, int writeFd) {
    char buf[64];
    for (int seq = 1; seq <= lines; ++seq) {
        int n = std::snprintf(buf, sizeof(buf), "worker-%d msg %d\n", idx, seq);
        writeAll(writeFd, buf, (size_t)n);   // 一行一次写完（writeAll 内部只在部分写时才续写）
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

    // —— 先装信号，再 fork：保证第一个子进程结束时处理函数已经就位 ——
    struct sigaction sa;
    sa.sa_handler = onChld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGCHLD, &sa, NULL) < 0) { std::perror("sigaction SIGCHLD"); return 1; }
    struct sigaction sb;
    sb.sa_handler = onStop;
    sigemptyset(&sb.sa_mask);
    sb.sa_flags = 0;
    if (sigaction(SIGTERM, &sb, NULL) < 0) { std::perror("sigaction SIGTERM"); return 1; }
    if (sigaction(SIGINT, &sb, NULL) < 0) { std::perror("sigaction SIGINT"); return 1; }

    int logFd = open(logPath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (logFd < 0) { std::perror("open logfile"); return 1; }

    int pipeFd[2];
    if (pipe(pipeFd) < 0) { std::perror("pipe"); return 1; }

    std::vector<pid_t> kids;
    for (int i = 0; i < workers; ++i) {
        pid_t pid = fork();
        if (pid < 0) { std::perror("fork"); return 1; }
        if (pid == 0) {
            // 关键一步：worker 先把信号处置恢复默认。
            // fork 会继承父进程的处置表——若照单全收，父进程转发来的 SIGTERM
            // 只会触发一份"父进程的 handler"，去置 worker 自己那份没人看的标志，
            // worker 将对关闭信号毫无反应（验收 B 第一版就死在这里）。
            signal(SIGTERM, SIG_DFL);
            signal(SIGINT, SIG_DFL);
            signal(SIGCHLD, SIG_DFL);
            close(pipeFd[0]);
            workerMain(i, lines, pipeFd[1]);
        }
        kids.push_back(pid);
    }
    close(pipeFd[1]);   // 父进程的写端副本立刻关：EOF 的前提

    // 读端改非阻塞：主循环要在"读数据 / 回收 / 响应关闭"之间轮转，不能卡死在 read 上
    int flags = fcntl(pipeFd[0], F_GETFL, 0);
    fcntl(pipeFd[0], F_SETFL, flags | O_NONBLOCK);

    std::vector<int> exitCodes((size_t)workers, -999);
    std::vector<char> reapedFlag((size_t)workers, 0);
    int reaped = 0;
    long totalLines = 0;
    bool stopping = false;
    bool eof = false;
    std::string pending;

    while (!eof || reaped < workers) {
        // 1) 回收：信号合并过，所以一次标志要排干
        if (g_childEvent) {
            g_childEvent = 0;
            for (;;) {
                int status = 0;
                pid_t p = waitpid(-1, &status, WNOHANG);
                if (p <= 0) break;
                for (int i = 0; i < workers; ++i) {
                    if (kids[(size_t)i] == p) {
                        reapedFlag[(size_t)i] = 1;
                        exitCodes[(size_t)i] = WIFEXITED(status)
                            ? WEXITSTATUS(status)
                            : -WTERMSIG(status);   // 被信号终止记为负信号号
                    }
                }
                ++reaped;
            }
        }

        // 2) 优雅关闭：第一次见标志时把 SIGTERM 转发给还活着的 worker
        if (g_stop && !stopping) {
            stopping = true;
            for (int i = 0; i < workers; ++i) {
                if (!reapedFlag[(size_t)i]) kill(kids[(size_t)i], SIGTERM);
            }
        }

        // 3) 读数据：还有多少读多少，完整行立刻落盘
        if (!eof) {
            char chunk[4096];
            ssize_t r = read(pipeFd[0], chunk, sizeof(chunk));
            if (r == 0) {
                eof = true;
            } else if (r > 0) {
                pending.append(chunk, (size_t)r);
                std::string::size_type pos;
                while ((pos = pending.find('\n')) != std::string::npos) {
                    std::string line = pending.substr(0, pos);
                    pending.erase(0, pos + 1);
                    std::string outLine = nowPrefix() + line + "\n";
                    writeAll(logFd, outLine.c_str(), outLine.size());
                    ++totalLines;
                }
            } else if (errno == EAGAIN) {
                usleep(2000);          // 暂时没数据，让出 CPU
            } else if (errno != EINTR) {
                std::perror("read pipe");
                eof = true;
            }
        } else {
            usleep(2000);              // 数据已读完，只剩等回收
        }
    }

    // 理论上 pending 应为空（worker 都整行写）；真有残行也如数落盘
    if (!pending.empty()) {
        std::string outLine = nowPrefix() + pending + "\n";
        writeAll(logFd, outLine.c_str(), outLine.size());
        ++totalLines;
    }

    // —— 收尾：日志收尾行 + ECHILD 自证 + stdout 三行 ——
    char tail[128];
    int tn = std::snprintf(tail, sizeof(tail), "collector shutdown: workers=%d lines=%ld\n",
                           workers, totalLines);
    std::string tailLine = nowPrefix() + std::string(tail, (size_t)tn);
    writeAll(logFd, tailLine.c_str(), tailLine.size());
    close(pipeFd[0]);
    close(logFd);

    int status = 0;
    errno = 0;
    pid_t p = waitpid(-1, &status, WNOHANG);
    bool noZombies = (p == -1 && errno == ECHILD);

    std::printf("total lines = %ld\n", totalLines);
    std::printf("worker exit codes:");
    for (int i = 0; i < workers; ++i) std::printf(" %d", exitCodes[(size_t)i]);
    std::printf("\n");
    if (noZombies) {
        std::printf("reaped %d workers, no zombies (ECHILD)\n", reaped);
    } else {
        std::printf("WARNING: leftover children (waitpid=%d errno=%d)\n", p, errno);
        return 1;
    }
    return 0;
}
