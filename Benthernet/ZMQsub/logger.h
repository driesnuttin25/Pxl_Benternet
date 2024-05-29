#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <mutex>
#include <ctime>
#include <iomanip>

class Logger {
public:
    enum class Level {
        INFO,
        WARNING,
        ERROR
    };

    // Log a message with a specified severity level
    static void log(Level level, const std::string& message);

private:
    static std::mutex logMutex;

    static std::string getCurrentTime();
    static std::string levelToString(Level level);
};

#endif // LOGGER_H
