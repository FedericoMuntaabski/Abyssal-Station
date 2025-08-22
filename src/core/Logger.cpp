#include "Logger.h"

#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <iomanip>

namespace core {

class LoggerImpl {
public:
    LoggerImpl() : consoleEnabled(true) {}

    bool setFile(const std::string& path) {
        std::lock_guard<std::mutex> lk(mutex);
        fileStream.open(path, std::ios::app);
        return fileStream.is_open();
    }

    void enableConsole(bool e) {
        std::lock_guard<std::mutex> lk(mutex);
        consoleEnabled = e;
    }

    void log(LogLevel level, const std::string& msg) {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        std::tm tm;
#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif

        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " ";
        switch (level) {
            case LogLevel::Info: oss << "[INFO] "; break;
            case LogLevel::Warning: oss << "[WARN] "; break;
            case LogLevel::Error: oss << "[ERROR] "; break;
        }
        oss << msg << '\n';

        std::lock_guard<std::mutex> lk(mutex);
        if (consoleEnabled) {
            if (level == LogLevel::Error) std::cerr << oss.str();
            else std::cout << oss.str();
        }
        if (fileStream.is_open()) {
            fileStream << oss.str();
            fileStream.flush();
        }
    }

private:
    std::ofstream fileStream;
    bool consoleEnabled;
    std::mutex mutex;
};


Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

Logger::Logger() = default;

Logger::~Logger() = default;

// single shared impl
static LoggerImpl& impl() {
    static LoggerImpl s_impl;
    return s_impl;
}

void Logger::enableConsole(bool enable) { impl().enableConsole(enable); }

bool Logger::setLogFile(const std::string& filepath) { return impl().setFile(filepath); }

void Logger::info(const std::string& msg) { impl().log(LogLevel::Info, msg); }

void Logger::warning(const std::string& msg) { impl().log(LogLevel::Warning, msg); }

void Logger::error(const std::string& msg) { impl().log(LogLevel::Error, msg); }

void Logger::log(LogLevel level, const std::string& msg) { impl().log(level, msg); }

} // namespace core
