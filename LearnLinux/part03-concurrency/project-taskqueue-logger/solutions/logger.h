#pragma once
// ============================================================================
// Logger —— 第三部分定型接口（毕业项目直接复用，签名不要改）
//
//   explicit Logger(const std::string& path, LogLevel minLevel = LogLevel::DEBUG);
//   void log(LogLevel level, const std::string& msg);
//   void debug/info/warn/error(const std::string& msg);
//   void setLevel(LogLevel level);  LogLevel level() const;
//   void stop();                     // 排空 + flush + 回收后端线程；幂等
//
// 语义定型：
//   - 构造即打开文件并启动后端线程；打开失败抛 std::runtime_error；
//   - 前端 log() 只做三件事：级别过滤、取时间戳、入队——格式化与写盘全在后端，
//     所以业务线程永远不会被磁盘 IO 堵住；
//   - 低于 minLevel 的日志在入队前就被丢弃（不占队列、不占磁盘）；
//   - stop() 之后再 log() 的消息被静默丢弃（关闭期与业务收尾经常交叠，
//     这里选择"丢弃"而不是"抛异常"，理由见任务书）；
//   - 落盘格式：[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] msg —— 时间戳取日志产生时刻。
// 结构：前端多线程 -> BlockingQueue（无界）-> 单后端线程格式化落盘。
//       单写者天然保证整行不交错（第 3、5 章的结论在这里变现）。
// ============================================================================
#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

#include "blocking_queue.h"

enum class LogLevel { DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3 };

class Logger {
public:
    explicit Logger(const std::string& path, LogLevel minLevel = LogLevel::DEBUG)
        : minLevel_(minLevel), stopped_(false), out_(path.c_str()) {
        if (!out_) throw std::runtime_error("Logger: cannot open " + path);
        worker_ = std::thread(&Logger::backendLoop, this);
    }

    ~Logger() { stop(); }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void log(LogLevel level, const std::string& msg) {
        // stateM_ 把 log 与 stop 串行化：要么赶在关门前入队（保证被写出），
        // 要么在关门后到达（按定型语义静默丢弃）——不存在"关门瞬间入队"的缝。
        std::lock_guard<std::mutex> lk(stateM_);
        if (stopped_) return;
        if (level < minLevel_) return;
        Item it;
        it.level = level;
        it.msg = msg;
        it.ts = std::chrono::system_clock::now();
        queue_.push(it);                      // 无界队列，push 不阻塞
    }

    void debug(const std::string& m) { log(LogLevel::DEBUG, m); }
    void info(const std::string& m) { log(LogLevel::INFO, m); }
    void warn(const std::string& m) { log(LogLevel::WARN, m); }
    void error(const std::string& m) { log(LogLevel::ERROR, m); }

    void setLevel(LogLevel level) {
        std::lock_guard<std::mutex> lk(stateM_);
        minLevel_ = level;
    }

    LogLevel level() const {
        std::lock_guard<std::mutex> lk(stateM_);
        return minLevel_;
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lk(stateM_);
            if (stopped_) return;            // 幂等：第二次调用直接返回
            stopped_ = true;
        }
        queue_.close();                       // 后端取完存量后 pop 返回 false
        if (worker_.joinable()) worker_.join();
    }

private:
    struct Item {
        LogLevel level;
        std::string msg;
        std::chrono::system_clock::time_point ts;
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
        Item it;
        while (queue_.pop(it)) {              // 唯一的写者：整行天然不交错
            std::time_t t = std::chrono::system_clock::to_time_t(it.ts);
            long ms = static_cast<long>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    it.ts.time_since_epoch()).count() % 1000);
            struct tm tmv;
            localtime_r(&t, &tmv);
            char line[64 + 1024];
            int n = std::snprintf(
                line, sizeof(line),
                "[%04d-%02d-%02d %02d:%02d:%02d.%03ld] [%s] ",
                tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday,
                tmv.tm_hour, tmv.tm_min, tmv.tm_sec, ms, levelName(it.level));
            out_.write(line, n);
            out_ << it.msg << '\n';
        }
        out_.flush();                         // 队列取空且已关闭：收尾刷盘
    }

    BlockingQueue<Item> queue_;
    LogLevel minLevel_;
    bool stopped_;
    mutable std::mutex stateM_;
    std::ofstream out_;
    std::thread worker_;
};
