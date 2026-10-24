// ex03：字节流语义 —— send_all / recv_all 循环封装
// TCP 是字节流，没有消息边界：send 返回"本次实际发出多少"、recv 返回"本次实际
// 收到多少"，两者都可能小于请求长度（哪怕对方一次发完）。正确姿势是循环，
// 直到凑够字节数、对端关闭（recv 返回 0）、或出错为止。
// 本例：客户端发 100000 字节模式数据（第 i 字节 = i % 251）；服务端循环收满，
// 独立算出同一模式的期望校验和并核对——字节数与校验和双精确才算过。
#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <future>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {

const int kBytes = 100000;

void die(const char* what) {
    std::perror(what);
    _exit(1);
}

// 发满 len 字节才返回；返回 false 表示对端关闭或出错。
bool sendAll(int fd, const char* buf, std::size_t len) {
    while (len > 0) {
        ssize_t n = send(fd, buf, len, 0);
        if (n < 0) {
            if (errno == EINTR) continue;    // 被信号打断：重发，不是失败
            return false;
        }
        buf += n;
        len -= static_cast<std::size_t>(n);
    }
    return true;
}

bool recvAll(int fd, char* buf, std::size_t len) {
    while (len > 0) {
        ssize_t n = recv(fd, buf, len, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (n == 0) return false;            // 对端关闭：数据没凑够
        buf += n;
        len -= static_cast<std::size_t>(n);
    }
    return true;
}

std::uint64_t patternSum(int n) {
    std::uint64_t s = 0;
    for (int i = 0; i < n; ++i) s += static_cast<unsigned char>(i % 251);
    return s;
}

void serverThread(std::promise<int> portPromise) {
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) die("socket");
    int reuse = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(0);
    if (bind(listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        die("bind");
    if (listen(listenFd, 1) < 0) die("listen");
    sockaddr_in bound;
    socklen_t len = sizeof(bound);
    if (getsockname(listenFd, reinterpret_cast<sockaddr*>(&bound), &len) < 0)
        die("getsockname");
    portPromise.set_value(ntohs(bound.sin_port));

    int conn = accept(listenFd, NULL, NULL);
    if (conn < 0) die("accept");
    std::vector<char> buf(kBytes);
    if (!recvAll(conn, buf.data(), buf.size())) die("recvAll");
    std::uint64_t got = 0;
    for (int i = 0; i < kBytes; ++i) got += static_cast<unsigned char>(buf[i]);
    std::printf("received %d bytes\n", kBytes);
    if (got == patternSum(kBytes))
        std::printf("checksum match\n");
    else
        std::printf("checksum MISMATCH: got %llu expected %llu\n",
                    static_cast<unsigned long long>(got),
                    static_cast<unsigned long long>(patternSum(kBytes)));
    close(conn);
    close(listenFd);
}

}  // namespace

int main() {
    std::promise<int> portPromise;
    std::future<int> portFuture = portPromise.get_future();
    std::thread server(serverThread, std::move(portPromise));
    int port = portFuture.get();

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) die("socket");
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(static_cast<std::uint16_t>(port));
    if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        die("connect");

    std::vector<char> buf(kBytes);
    for (int i = 0; i < kBytes; ++i) buf[i] = static_cast<char>(i % 251);
    if (!sendAll(fd, buf.data(), buf.size())) die("sendAll");
    close(fd);
    server.join();
    return 0;
}
