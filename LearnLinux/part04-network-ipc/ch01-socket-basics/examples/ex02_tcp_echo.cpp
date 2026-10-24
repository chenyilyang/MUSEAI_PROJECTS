// ex02：TCP 最小闭环 —— 服务端线程 + 客户端同进程，127.0.0.1 动态端口
// 服务端四步：socket → bind → listen → accept；客户端两步：socket → connect。
// 动态端口：bind 时端口填 0（让内核挑），再用 getsockname 把实际端口读出来，
// 经 std::promise 交给主线程——验收不依赖任何固定端口，也就不会被别的进程占用打断。
// 坦白一个简化：这里的收发各只做一次 send/recv。消息只有 12 字节、走回环，
// 实践中一次收发就能到齐；但这不是保证——正式写法是 ex03 的 send_all/recv_all。
#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <future>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

static void die(const char* what) {
    std::perror(what);
    _exit(1);                                // 演示程序：出错即退，不做恢复
}

static void serverThread(std::promise<int> portPromise) {
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) die("socket");

    int reuse = 1;                           // 允许 TIME_WAIT 的地址被立刻重用
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);   // 127.0.0.1，只走回环
    addr.sin_port = htons(0);                        // 端口 0：内核分配
    if (bind(listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        die("bind");
    if (listen(listenFd, 1) < 0) die("listen");

    sockaddr_in bound;                       // 读回内核实际分配的端口
    socklen_t len = sizeof(bound);
    if (getsockname(listenFd, reinterpret_cast<sockaddr*>(&bound), &len) < 0)
        die("getsockname");
    portPromise.set_value(ntohs(bound.sin_port));

    int conn = accept(listenFd, NULL, NULL);
    if (conn < 0) die("accept");
    char buf[64];
    ssize_t n = recv(conn, buf, sizeof(buf), 0);
    if (n < 0) die("recv");
    if (send(conn, buf, static_cast<std::size_t>(n), 0) < 0) die("send");
    close(conn);
    close(listenFd);
}

int main() {
    std::promise<int> portPromise;
    std::future<int> portFuture = portPromise.get_future();
    std::thread server(serverThread, std::move(portPromise));
    int port = portFuture.get();             // 等到 listen 就绪才连，避免竞态

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) die("socket");
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(static_cast<std::uint16_t>(port));
    if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        die("connect");

    const char* msg = "hello socket";
    if (send(fd, msg, std::strlen(msg), 0) < 0) die("send");
    char buf[64];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n < 0) die("recv");
    buf[n] = '\0';
    close(fd);
    server.join();

    if (std::strcmp(buf, msg) == 0) {
        std::printf("echo OK: %s\n", buf);
        return 0;
    }
    std::printf("echo MISMATCH: got [%s]\n", buf);
    return 1;
}
