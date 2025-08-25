#include "Logger.h"

#include <iostream>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#endif

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
#ifdef _WIN32
            const std::string out = oss.str();
            HANDLE h = (level == LogLevel::Error) ? GetStdHandle(STD_ERROR_HANDLE) : GetStdHandle(STD_OUTPUT_HANDLE);
            bool wrote = false;
            if (h != INVALID_HANDLE_VALUE) {
                int wlen = MultiByteToWideChar(CP_UTF8, 0, out.c_str(), -1, nullptr, 0);
                if (wlen > 0) {
                    std::wstring wstr(static_cast<size_t>(wlen), L'\0');
                    MultiByteToWideChar(CP_UTF8, 0, out.c_str(), -1, &wstr[0], wlen);
                    DWORD written = 0;
                    // Try WriteConsoleW even if handle might be redirected; if it fails, we'll fallback
                    if (WriteConsoleW(h, wstr.c_str(), static_cast<DWORD>(wstr.size() - 1), &written, nullptr)) {
                        wrote = true;
                    }
                }
            }
            if (!wrote) {
                // Fallback: write UTF-8 bytes directly to stdout/stderr.
                // Modern terminals (Windows Terminal, VSCode) expect UTF-8 and will render accents correctly.
                if (level == LogLevel::Error) std::cerr << out;
                else std::cout << out;
            }
            if (level == LogLevel::Error) std::cerr.flush();
            else std::cout.flush();
#else
            if (level == LogLevel::Error) std::cerr << oss.str();
            else std::cout << oss.str();
            if (level == LogLevel::Error) std::cerr.flush();
            else std::cout.flush();
#endif
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
