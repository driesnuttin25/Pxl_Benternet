#include "logger.h"

std::mutex Logger::logMutex;
std::ofstream Logger::logFile;

void Logger::init(const std::string& filename) {
    std::lock_guard<std::mutex> guard(logMutex);
    logFile.open(filename, std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

void Logger::close() {
    std::lock_guard<std::mutex> guard(logMutex);
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::log(Level level, const std::string& message) {
    std::lock_guard<std::mutex> guard(logMutex);
    std::string logMessage = "[" + getCurrentTime() + "] [" + levelToString(level) + "] " + message;

    // Log to console
    std::cout << logMessage << std::endl;

    // Log to file if open
    if (logFile.is_open()) {
        logFile << logMessage << std::endl;
    }
}

std::string Logger::getCurrentTime() {
    auto now = std::time(nullptr);
    std::tm now_tm;
    localtime_s(&now_tm, &now);  // Or localtime_r in POSIX systems
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
