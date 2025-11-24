#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <filesystem>

Logger::~Logger() {
    close();
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::init(const std::string& filename, size_t maxFileSize, size_t maxFiles) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        logFile_.close();
    }
    logFilename_ = filename;
    maxFileSize_ = maxFileSize;
    maxFiles_ = maxFiles;
    logFile_.open(filename, std::ios::app);
    initialized_ = true;
    log(Level::INFO, "Logger initialized");
}

void Logger::setMinLevel(Level level) {
    std::lock_guard<std::mutex> lock(mutex_);
    minLevel_ = level;
}

void Logger::log(Level level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_ || level < minLevel_) {
        return;
    }

    // Check file size and rotate if needed
    if (maxFileSize_ > 0 && logFile_.tellp() > static_cast<std::streampos>(maxFileSize_)) {
        rotateLogFile();
    }

    std::string logLine = "[" + getCurrentTime() + "] [" + levelToString(level) + "] " + message;
    
    logFile_ << logLine << std::endl;
    logFile_.flush();

    // Also output ERROR to stderr
    if (level == Level::ERROR) {
        std::cerr << logLine << std::endl;
    }
}

void Logger::close() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (logFile_.is_open()) {
        logFile_.close();
        initialized_ = false;
    }
}

void Logger::rotateLogFile() {
    logFile_.close();
    
    try {
        namespace fs = std::filesystem;
        fs::path logPath(logFilename_);
        
        // Rotate existing files
        for (size_t i = maxFiles_ - 1; i > 0; --i) {
            fs::path oldFile = logPath.parent_path() / (logPath.stem().string() + "." + std::to_string(i) + logPath.extension().string());
            fs::path newFile = logPath.parent_path() / (logPath.stem().string() + "." + std::to_string(i + 1) + logPath.extension().string());
            
            if (fs::exists(oldFile)) {
                if (fs::exists(newFile)) {
                    fs::remove(newFile);
                }
                fs::rename(oldFile, newFile);
            }
        }
        
        // Move current log to .1
        fs::path firstBackup = logPath.parent_path() / (logPath.stem().string() + ".1" + logPath.extension().string());
        if (fs::exists(logPath)) {
            if (fs::exists(firstBackup)) {
                fs::remove(firstBackup);
            }
            fs::rename(logPath, firstBackup);
        }
    } catch (const std::exception& e) {
        std::cerr << "Log rotation failed: " << e.what() << std::endl;
    }
    
    logFile_.open(logFilename_, std::ios::trunc);
}

std::string Logger::levelToString(Level level) const {
    switch (level) {
        case Level::DEBUG: return "DEBUG";
        case Level::INFO: return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string Logger::getCurrentTime() const {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}
