/**
 * @file Logger.cpp
 * @brief Thread-safe logging utility with console styling and file output for SpoofEye.
 * 
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "utils/Logger.hpp"
#include "constants.hpp"

#include <syslog.h>

std::string Logger::logFilePath_;
bool Logger::useStyle_ = true;
std::mutex Logger::mutex_;

void Logger::init(const std::string& filePath, bool useStyle) {
    std::lock_guard<std::mutex> lock(mutex_);
    logFilePath_.clear();
    useStyle_ = useStyle;

    if (filePath.empty()) {
        // fallback to syslog + console only
        return;
    }

    logFilePath_ = filePath;

    auto parent = std::filesystem::path(filePath).parent_path();
    if (!parent.empty()) {
        // Create parent directories if they do not exist
        std::filesystem::create_directories(parent);
    }
}

std::string Logger::formatOutput(const std::string& message, LogType type, bool showDetails, bool useStyle, const std::string& prefix) {
    std::ostringstream oss;

    if (showDetails) {
        oss << "[" << std::left << std::setw(TIME_FIELD_WIDTH) << currentDateTime() << "] "
            << std::left << std::setw(TYPE_FIELD_WIDTH) << typeToString(type);
    }

    if (!prefix.empty()) {
        oss << "[" << prefix << "]: ";
    }
    oss << message;

    std::string output = oss.str();

    if (useStyle) {
        std::string colorCode;
        switch (type) {
            case LogType::WARNING:
            case LogType::DEBUG:     colorCode = "\033[33m"; break; // yellow
            case LogType::ERROR:     
            case LogType::CRITICAL:  colorCode = "\033[31m"; break; // red
            default:                 colorCode = "\033[0m";  break;
        }
        output = colorCode + "\033[1m" + output + "\033[0m";
    }

    return output;
}

void Logger::log(const std::string& message, LogType type, const std::string& prefix) {
    std::lock_guard<std::mutex> lock(mutex_);

    std::string writeOutput = formatOutput(message, type, true, false, prefix);
    std::string syslogOutput = formatOutput(message, type, false, false, prefix);
    std::string consoleOutput = formatOutput(message, type, true, useStyle_, prefix);

    // --- File logging ---
    if (logFilePath_.empty() != true && logFilePath_ != "") {
        std::ofstream logfile(logFilePath_, std::ios::app);

        if (logfile.is_open()) {
            logfile << writeOutput << std::endl;
            logfile.close();
        } else {
            std::cerr << formatOutput("Failed to open log file: " + logFilePath_, LogType::ERROR, false, useStyle_) << std::endl;
        }
    }

    // --- Syslog logging ---
    int syslogPriority;
    switch (type) {
        case LogType::INFO:     syslogPriority = LOG_INFO; break;
        case LogType::DEBUG:    syslogPriority = LOG_DEBUG; break;
        case LogType::WARNING:  syslogPriority = LOG_WARNING; break;
        case LogType::ERROR:    syslogPriority = LOG_ERR; break;
        case LogType::CRITICAL: syslogPriority = LOG_CRIT; break;
        default:                syslogPriority = LOG_INFO; break;
    }

    // Open syslog
    openlog(SOFTWARE_COMMAND, LOG_PID | LOG_CONS, LOG_USER);
    syslog(syslogPriority, "%s", syslogOutput.c_str());
    closelog();

    // --- Console output ---
    if (type == LogType::WARNING || type == LogType::ERROR || type == LogType::CRITICAL) {
        std::cerr << consoleOutput << std::endl;
    } else {
        std::cout << consoleOutput << std::endl;
    }
}

void Logger::print(const std::string& message, LogType type, const std::string& prefix) {
    std::string output = formatOutput(message, type, false, useStyle_, prefix);
    if (type == LogType::WARNING || type == LogType::ERROR || type == LogType::CRITICAL) {
        std::cerr << output << std::endl;
    } else {
        std::cout << output << std::endl;
    }
}

std::string Logger::currentDateTime() {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&timeT), "%Y-%m-%dT%H:%M:%S")
       << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z";
    return ss.str();
}

std::string Logger::typeToString(LogType type) {
    switch (type) {
        case LogType::INFO:      return "(INFO)";
        case LogType::DEBUG:   return "(DEBUG)";
        case LogType::WARNING:   return "(WARNING)";
        case LogType::ERROR:     return "(ERROR)";
        case LogType::CRITICAL:  return "(CRITICAL)";
        default:                 return "(DEFAULT)";
    }
}
