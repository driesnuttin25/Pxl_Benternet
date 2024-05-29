#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <ctime>
#include <iomanip>

/**********************************
*  Logger Service class
**********************************/

class Logger {
public:
    enum class Level {
        INFO,
        WARNING,
        ERROR
    };

    // Log a message with a specified severity level
    static void log(Level level, const std::string& message);

    // Initialize the logger with a file
    static void init(const std::string& filename);

    // Close the log file
    static void close();

private:
    static std::mutex logMutex;
    static std::ofstream logFile;

    static std::string getCurrentTime();
    static std::string levelToString(Level level);
};

#endif // LOGGER_H
