/**
 * @file Logger.hpp
 * @brief Thread-safe logging utility with console styling and file output for SpoofEye.
 * 
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <sstream>

/**
 * @class Logger
 * @brief Provides logging to console and file with optional color formatting.
 */
class Logger {
public:
    /** Log severity levels */
    enum class LogType { DEFAULT, INFO, DEBUG, WARNING, ERROR, CRITICAL };

    /**
     * @brief Initialize the logger with a file path and optional console styling.
     * @param filePath Path to log file.
     * @param useStyle Enable console color formatting (default: true).
     */
    static void init(const std::string& filePath, bool useStyle = true);

    /**
     * @brief Log a message to file, syslog and console.
     * @param message Message text.
     * @param type Log severity.
     * @param prefix Optional prefix for the message (e.g., module name).
     */
    static void log(const std::string& message,
                    LogType type = LogType::DEFAULT,
                    const std::string& prefix = "");

    /**
     * @brief Print a message to console only, optionally styled.
     * @param message Message text.
     * @param type Log severity.
     * @param prefix Optional prefix for the message.
     */
    static void print(const std::string& message,
                      LogType type = LogType::DEFAULT,
                      const std::string& prefix = "");

private:
    static std::string currentDateTime();
    static std::string typeToString(LogType type);
    static std::string formatOutput(const std::string& message,
                                    LogType type,
                                    bool showDetails = false,
                                    bool useStyle = false,
                                    const std::string& prefix = "");

    static std::string logFilePath_;
    static bool useStyle_;
    static std::mutex mutex_;

    // Field widths for formatted output
    static constexpr int TIME_FIELD_WIDTH = 24;   ///< ISO8601 timestamp width
    static constexpr int TYPE_FIELD_WIDTH = 12;   ///< Log type field width
};
