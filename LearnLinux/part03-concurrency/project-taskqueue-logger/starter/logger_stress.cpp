// logger_stress —— Logger 验收程序（starter 与 solutions 共用同一份）
// 用法：logger_stress <日志路径> <生产者数> <每生产者条数>
// 每个生产者按 seq 循环打五种消息：seq%5 == 0:DEBUG 1:INFO 2:WARN 3:ERROR 4:INFO，
// 消息体固定为 "p<生产者> seq <序号>"。Logger 最低级别 INFO：
// DEBUG 应被全部过滤，其余每条恰好落盘一次——由 tests/verify_log.py 逐行核对。
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>
#include <vector>

#include "logger.h"

static LogLevel levelFor(int seq) {
    switch (seq % 5) {
        case 0: return LogLevel::DEBUG;
        case 2: return LogLevel::WARN;
        case 3: return LogLevel::ERROR;
        default: return LogLevel::INFO;      // 1 与 4
    }
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::fprintf(stderr, "usage: logger_stress <path> <producers> <per>\n");
        return 2;
    }
    const std::string path = argv[1];
    const int producers = std::atoi(argv[2]);
    const int per = std::atoi(argv[3]);

    Logger logger(path, LogLevel::INFO);
    std::vector<std::thread> ts;
    for (int p = 0; p < producers; ++p) {
        ts.push_back(std::thread([&, p] {
            for (int s = 0; s < per; ++s) {
                char buf[64];
                std::snprintf(buf, sizeof(buf), "p%d seq %d", p, s);
                logger.log(levelFor(s), buf);
            }
        }));
    }
    for (std::size_t i = 0; i < ts.size(); ++i) ts[i].join();
    logger.stop();                            // 排空 + 落盘 + 回收后端线程
    std::printf("stress done: producers = %d, per = %d\n", producers, per);
    return 0;
}
