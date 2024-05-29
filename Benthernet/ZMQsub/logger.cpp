#include "logger.h"

std::mutex Logger::logMutex;

void Logger::log(Level level, const std::string& message) {
    std::lock_guard<std::mutex> guard(logMutex);
    std::cout << "[" << getCurrentTime() << "] [" << levelToString(level) << "] " << message << std::endl;
}

std::string Logger::getCurrentTime() {
    auto now = std::time(nullptr);
    std::tm now_tm;
    localtime_s(&now_tm, &now);
    std::ostringstream oss;
    oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::levelToString(Level level) {
    switch (level) {
    case Level::INFO: return "INFO";
    case Level::WARNING: return "WARNING";
    case Level::ERROR: return "ERROR";
    default: return "UNKNOWN";
    }
}
