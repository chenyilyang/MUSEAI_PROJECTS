#pragma once
// ============================================================================
// 【骨架】Logger —— 形状对了，四个关键语义没实现（TODO）：
//   1. 级别过滤缺失：log() 不看 minLevel_，什么都往队列里塞；
//   2. stop() 不排空：置个标志就 join，后端线程一见标志就走人，
//      队列里没写完的日志整批丢失（尾部丢失——验收最看重的一条）；
//   3. 时间戳在后端写盘时才取，不是日志产生时刻（并发下顺序与时间对不上）；
//   4. log() 与 stop() 之间没有协调：stop 之后再 log 的行为未定义。
// 你的任务：按任务书规格补齐这四点。后端用本项目的 BlockingQueue 是顺路的解法。
// ============================================================================
#include <atomic>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

enum class LogLevel { DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3 };

class Logger {
public:
    explicit Logger(const std::string& path, LogLevel minLevel = LogLevel::DEBUG)
        : minLevel_(minLevel), stopFlag_(false), out_(path.c_str()) {
        worker_ = std::thread(&Logger::backendLoop, this);
    }

    ~Logger() { stop(); }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(LogLevel level, const std::string& msg) {
        // TODO 1：先按 level 与 minLevel_ 过滤，被过滤的连队都不该入。
        Item it;
        it.level = level;
        it.msg = msg;
        std::lock_guard<std::mutex> lk(m_);
        q_.push(it);
    }

    void debug(const std::string& m) { log(LogLevel::DEBUG, m); }
    void info(const std::string& m) { log(LogLevel::INFO, m); }
    void warn(const std::string& m) { log(LogLevel::WARN, m); }
    void error(const std::string& m) { log(LogLevel::ERROR, m); }

    void setLevel(LogLevel level) { minLevel_ = level; }
    LogLevel level() const { return minLevel_; }

    void stop() {
        // TODO 2/4：应先停收、再让后端把队列写空、flush、最后 join；且幂等。
        stopFlag_.store(true);
        if (worker_.joinable()) worker_.join();
    }

private:
    struct Item {
        LogLevel level;
        std::string msg;
    };

    static const char* levelName(LogLevel lv) {
        switch (lv) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARN: return "WARN";
            case LogLevel::ERROR: return "ERROR";
        }
        return "?";
    }

    void backendLoop() {
        for (;;) {
            if (stopFlag_.load()) return;    // TODO 2：见标志就走，队列存量全丢
            Item it;
            bool has = false;
            {
                std::lock_guard<std::mutex> lk(m_);
                if (!q_.empty()) {            // 空转轮询：正解应睡在队列上等
                    it = q_.front();
                    q_.pop();
                    has = true;
                }
            }
            if (!has) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            // TODO 3：时间戳在这里取已经晚了——这是写盘时刻，不是产生时刻。
            std::time_t t = std::time(NULL);
            struct tm tmv;
            localtime_r(&t, &tmv);
            char ts[32];
            std::strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tmv);
            out_ << "[" << ts << ".000] [" << levelName(it.level) << "] "
                 << it.msg << "\n";
        }
    }

    LogLevel minLevel_;
    std::atomic<bool> stopFlag_;
    std::ofstream out_;
    std::queue<Item> q_;
    std::mutex m_;
    std::thread worker_;
};
