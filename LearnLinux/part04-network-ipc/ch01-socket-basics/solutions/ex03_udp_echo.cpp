// 练习 3 参考答案：UDP 版 echo —— 对比 TCP 版，少了 listen 与 accept 两步
// UDP 是无连接的数据报：服务端不需要 listen/accept，客户端不需要 connect；
// 双方都用 sendto/recvfrom，指明对端地址即发即收。代价也在这里：
// 没有连接状态，就没有重传、没有顺序、没有流控——回环上测不出来，
// 真实网络上全都会出现（第 2 章会把这三点摊开讲）。
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <future>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

namespace {

void die(const char* what) {
    std::perror(what);
    _exit(1);
}

void serverThread(std::promise<int> portPromise) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);   // SOCK_DGRAM：数据报
    if (fd < 0) die("socket");
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(0);
    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        die("bind");
    sockaddr_in bound;
    socklen_t len = sizeof(bound);
    if (getsockname(fd, reinterpret_cast<sockaddr*>(&bound), &len) < 0)
        die("getsockname");
    portPromise.set_value(ntohs(bound.sin_port));

    char buf[64];
    sockaddr_in peer;
    socklen_t peerLen = sizeof(peer);
    ssize_t n = recvfrom(fd, buf, sizeof(buf), 0,
                         reinterpret_cast<sockaddr*>(&peer), &peerLen);
    if (n < 0) die("recvfrom");
    if (sendto(fd, buf, static_cast<std::size_t>(n), 0,
               reinterpret_cast<sockaddr*>(&peer), peerLen) < 0)
        die("sendto");
    close(fd);
}

}  // namespace

int main() {
    std::promise<int> portPromise;
    std::future<int> portFuture = portPromise.get_future();
    std::thread server(serverThread, std::move(portPromise));
    int port = portFuture.get();

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) die("socket");
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(static_cast<std::uint16_t>(port));

    const char* msg = "hello udp";
    if (sendto(fd, msg, std::strlen(msg), 0,
               reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        die("sendto");
    char buf[64];
    ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, NULL, NULL);
    if (n < 0) die("recvfrom");
    buf[n] = '\0';
    close(fd);
    server.join();

    if (std::strcmp(buf, msg) == 0) {
        std::printf("udp echo OK: %s\n", buf);
        return 0;
    }
    std::printf("udp echo MISMATCH: got [%s]\n", buf);
    return 1;
}
