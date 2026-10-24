// ex01：字节序 —— 主机字节序 vs 网络字节序
// 判定主机字节序：把 0x0001 写进内存，看第一个字节是 01（小端）还是 00（大端）。
// 关键结论（与主机无关、验收就断言这两条）：
//   htonl(0x12345678) 在内存里的字节恒为 12 34 56 78（这就是"网络字节序"的定义）；
//   ntohl(htonl(x)) == x 在任何主机上都成立——转换函数在大小端主机上做相反的事，
//   恰好抵消，所以"先转过去再转回来"永远是恒等操作。
#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>

int main() {
    std::uint16_t probe = 0x0001;
    bool little = (*reinterpret_cast<unsigned char*>(&probe) == 0x01);
    std::printf("host byte order: %s-endian\n", little ? "little" : "big");

    const std::uint32_t v = 0x12345678;
    const std::uint32_t net = htonl(v);
    const unsigned char* b = reinterpret_cast<const unsigned char*>(&net);
    std::printf("network bytes of 0x12345678: %02x %02x %02x %02x\n",
                b[0], b[1], b[2], b[3]);

    const std::uint32_t back = ntohl(net);
    std::printf("round-trip: 0x12345678 -> htonl -> ntohl = 0x%08x\n", back);
    return back == v ? 0 : 1;
}
