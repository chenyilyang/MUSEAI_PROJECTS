// 练习 2 参考答案：一问一答多轮回显 —— 长度前缀成帧（第 2 章的预演）
// 裸流上无法区分"三条消息"的边界：服务端若按 recv 次数切分，迟早在粘包上翻车。
// 这里先用最简单的成帧法垫底：每条消息前加 4 字节长度（网络字节序），
// 收发双方都按"先读长度、再按长度读载荷"的状态机走——这是第 2 章的正式主题。
#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <future>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {

void die(const char* what) {
    std::perror(what);
    _exit(1);
}

bool sendAll(int fd, const char* buf, std::size_t len) {
    while (len > 0) {
        ssize_t n = send(fd, buf, len, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
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
        if (n == 0) return false;
        buf += n;
        len -= static_cast<std::size_t>(n);
    }
    return true;
}

bool sendFrame(int fd, const std::string& msg) {
    std::uint32_t len = htonl(static_cast<std::uint32_t>(msg.size()));
    return sendAll(fd, reinterpret_cast<const char*>(&len), 4) &&
           sendAll(fd, msg.data(), msg.size());
}

// 读一帧；返回 false 表示对端关闭或出错（调用方以此结束循环）。
bool recvFrame(int fd, std::string* out) {
    std::uint32_t netLen = 0;
    if (!recvAll(fd, reinterpret_cast<char*>(&netLen), 4)) return false;
    std::uint32_t len = ntohl(netLen);
    out->resize(len);
    return recvAll(fd, &(*out)[0], len);
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
    std::string msg;
    while (recvFrame(conn, &msg)) {
        if (!sendFrame(conn, msg)) die("sendFrame");
    }
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

    const char* msgs[3] = {"one", "two", "three"};
    int ok = 0;
    for (int i = 0; i < 3; ++i) {
        if (!sendFrame(fd, msgs[i])) die("sendFrame");
        std::string back;
        if (recvFrame(fd, &back) && back == msgs[i]) ++ok;
    }
    close(fd);
    server.join();
    std::printf("multi-round echo OK: %d/3\n", ok);
    return ok == 3 ? 0 : 1;
}
