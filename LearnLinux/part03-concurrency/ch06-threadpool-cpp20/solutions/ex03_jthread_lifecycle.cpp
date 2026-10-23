// 练习 3 参考答案：用 jthread 重写第 1 章的参数传递示例【-std=c++20 构建】
// 第 1 章 ex02 用 std::thread 演示参数三式（值参 / std::ref / 指针），
// 收尾必须手工 join：一旦忘了、或中途抛异常绕过 join，析构直接 terminate。
// jthread 版的差别不在参数（规则完全一样），在生命周期：
//   作用域结束自动 join，第 1 章要盯着的 joinable 管理整段消失；
//   另外白得一个 stop_token 首参——本例用不上取消，只是让它露个脸。
#include <cstdio>
#include <stop_token>
#include <thread>
#include <utility>

static void worker(std::stop_token /*st*/, int x, int& y, int* z) {
    y = 99;                                   // ref：改的是主线程的 y（靠 std::ref 传入）
    *z = 7;                                   // 指针：改的是主线程的 z 所指
    std::printf("params: x = %d, y = %d, z = %d\n", x, y, *z);
}

int main() {
    int y = 0;
    int z = 0;
    {
        std::jthread t(worker, 1, std::ref(y), &z);
        // 没有 join，没有 joinable 判断——作用域结束时析构自动 join。
    }
    std::printf("main: scope ended, jthread joined automatically\n");
    std::printf("main: y = %d, z = %d (written by worker)\n", y, z);
    return (y == 99 && z == 7) ? 0 : 1;
}
