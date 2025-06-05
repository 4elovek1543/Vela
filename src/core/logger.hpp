#pragma once

#include <string>
#include <fstream> 


enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};


class Logger {
public:
    static void init(const std::string &log_file_path="vela.log");
    static bool set_log_file(std::string log_file_path);

    static void set_level(LogLevel level);

    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
private:
    static LogLevel current_level;
    static std::ofstream log_file;

    static void log(LogLevel level, const std::string& message);
};