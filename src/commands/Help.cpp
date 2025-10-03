/**
 * @file Help.cpp
 * @brief Implementation of the Help command for SpoofEye.
 *
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "commands/Help.hpp"
#include "utils/Logger.hpp"
#include "constants.hpp"

#include <string>

namespace commands {

Help::Help(const Init& init)
    : cmdManager_(init) {}

void Help::execute(const std::string&) {
    // Print software information
    Logger::print(
        std::string(SOFTWARE_NAME) + " v" + SOFTWARE_VERSION + "\n"
        "Author: " + SOFTWARE_AUTHOR + "\n"
        "License: " + SOFTWARE_LICENSE + "\n"
        "Copyright (c) " + SOFTWARE_COPYRIGHT_DATE + "\n\n"
    );

    // Print usage header
    Logger::print("Usage: " + std::string(SOFTWARE_COMMAND) + " COMMAND");
    Logger::print("Available commands:");
    
    // Print all commands from Init
    cmdManager_.printCommands();

    // Print usage examples
    Logger::print("\nUsage examples:");
    Logger::print("  " + std::string(SOFTWARE_COMMAND) + " --version");
    Logger::print("  " + std::string(SOFTWARE_COMMAND) + " --config-path /path/to/config.ini");
    Logger::print("  " + std::string(SOFTWARE_COMMAND) + " --help");
}

} // namespace commands
