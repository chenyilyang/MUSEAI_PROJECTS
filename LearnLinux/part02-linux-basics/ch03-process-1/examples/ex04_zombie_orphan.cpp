// ex04_zombie_orphan.cpp —— 僵尸与孤儿：亲手制造，再亲手收拾
// 运行时另开一个终端，按提示用 ps 观察。
// 编译基线：C++11
#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    // —— 第一幕：僵尸 ——
    // 子进程立刻退出，父进程故意不回收、先睡 10 秒：这段时间里子进程就是僵尸。
    pid_t z = fork();
    if (z == 0) {
        _exit(0);                      // 子进程的"一生"已经结束
    }
    std::printf("[zombie] child %d has exited but I won't reap it for 10s.\n", z);
    std::printf("[zombie] check now: ps -o pid,stat,cmd -p %d   (STAT should be Z)\n", z);
    std::fflush(stdout);
    sleep(10);
    int status = 0;
    waitpid(z, &status, 0);            // 睡醒了把尸收掉，僵尸消失
    std::printf("[zombie] reaped child %d. Zombie gone.\n", z);
    std::fflush(stdout);

    // —— 第二幕：孤儿 ——
    // 父进程打印完立刻退出，子进程睡 2 秒后报告自己"被谁收养了"。
    pid_t o = fork();
    if (o == 0) {
        sleep(2);                      // 醒来时父亲已经不在了
        std::printf("[orphan] my pid = %d, my new parent = %d\n", getpid(), getppid());
        std::fflush(stdout);
        _exit(0);
    }
    std::printf("[orphan] parent exiting now; child %d will be adopted.\n", o);
    std::fflush(stdout);
    return 0;                          // 父进程就此结束，不等子进程
}
