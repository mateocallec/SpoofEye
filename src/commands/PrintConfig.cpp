/**
 * @file PrintConfig.cpp
 * @brief Implementation of the PrintConfig command for SpoofEye.
 *
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "commands/PrintConfig.hpp"
#include "utils/Logger.hpp"
#include "constants.hpp"

#include <string>

namespace commands {

bool PrintConfig::print_config = false;

PrintConfig::PrintConfig() = default;

void PrintConfig::execute(const std::string&) {
    print_config = true;
}

void PrintConfig::printConfiguration(const Config& cfg) {
    // Print software information header
    Logger::print(
        std::string(SOFTWARE_NAME) + " v" + std::string(SOFTWARE_VERSION) + "\n"
        + "Author: " + std::string(SOFTWARE_AUTHOR) + "\n"
        + "License: " + std::string(SOFTWARE_LICENSE) + "\n"
        + "Copyright (c) " + std::string(SOFTWARE_COPYRIGHT_DATE) + "\n\n"
    );

    // Print configuration summary
    Logger::print(cfg.summary(), Logger::LogType::INFO);
}

} // namespace commands
