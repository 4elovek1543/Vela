#include "logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <unistd.h>


LogLevel Logger::current_level = LogLevel::INFO;
std::ofstream Logger::log_file;


void Logger::init(const std::string &log_file_path) {
    if (!Logger::set_log_file(log_file_path)) {
        std::cerr << "Failed to init Logger" << std::endl;
    }
}

bool Logger::set_log_file(const std::string &log_file_path) {
    if (log_file.is_open()) {
        log_file.close();
    }
    log_file.open(log_file_path, std::ios::app);
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file! (filepath" << log_file_path << ")" << std::endl;
        return false;
    }
    return true;
}

void Logger::set_level(LogLevel level) {
    Logger::current_level = level;
}

void Logger::log(LogLevel level, const std::string &message) {
    if (level < current_level) return;

    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto time = std::put_time(std::localtime(&now), "%Y-%m-%d %H:%M:%S");

    std::string level_str = "";
    std::string color_str = "\033[0m";
    switch (level) {
        case LogLevel::DEBUG: level_str = "DEBUG"; break;
        case LogLevel::INFO: level_str = "INFO"; break;
        case LogLevel::WARNING: level_str = "WARNING"; color_str = "\033[33m"; break;
        case LogLevel::ERROR: level_str = "ERROR"; color_str = "\033[31m"; break;
    }

    std::ostringstream oss;
    oss << time << " [" << level_str << "] " << message << std::endl;

    if (isatty(fileno(stdout))) {
        std::cout << color_str << oss.str() << "\033[0m";
    } else {
        std::cout << oss.str();
    }

    if (log_file.is_open()) {
        log_file << oss.str();
        log_file.flush();
    }
}

void Logger::debug(const std::string &message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string &message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string &message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string &message) {
    log(LogLevel::ERROR, message);
}
