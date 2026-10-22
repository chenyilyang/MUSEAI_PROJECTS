// ex02_fork_exec.cpp —— exec：映像整体替换，原来的代码"不复存在"
// 编译基线：C++11
#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    pid_t pid = fork();
    if (pid < 0) { std::perror("fork"); return 1; }

    if (pid == 0) {
        // 这行没有换行、没有 fflush：内容只躺在 stdio 缓冲里。
        // exec 替换映像时缓冲随旧映像一起消失——它永远不会被打印出来。
        std::printf("this line never appears");
        execl("/bin/echo", "echo", "hello from exec", (char*)NULL);
        // 能执行到这里，说明 exec 失败了（映像没换成）
        std::perror("execl");
        _exit(127);   // 子进程退出用 _exit：不冲刷继承来的 stdio 缓冲，不跑析构
    }

    int status = 0;
    waitpid(pid, &status, 0);
    std::printf("parent: child finished\n");
    return 0;
}
