#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <memory>

namespace core {

enum class LogLevel { Info, Warning, Error };

class Logger {
public:
    // Get singleton instance
    static Logger& instance();

    // Configure outputs
    void enableConsole(bool enable);
    bool setLogFile(const std::string& filepath); // returns true if opened

    // Logging API
    void info(const std::string& msg);
    void warning(const std::string& msg);
    void error(const std::string& msg);

    // Non-copyable
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    ~Logger();

    void log(LogLevel level, const std::string& msg);
};

} // namespace core

#endif // LOGGER_H
