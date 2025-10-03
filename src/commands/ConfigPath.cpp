/**
 * @file ConfigPath.cpp
 * @brief Implementation of the ConfigPath command for SpoofEye.
 *
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "commands/ConfigPath.hpp"
#include "utils/Logger.hpp"

#include <cstdlib>
#include <iostream>

namespace commands {

ConfigPath::ConfigPath(std::string& configPathRef)
    : configPath_(configPathRef) {}

void ConfigPath::execute(const std::string& arg) {
    if (arg.empty()) {
        Logger::print(
            "Error: --config-path requires a path argument.",
            Logger::LogType::ERROR
        );
        std::exit(2);
    }

    configPath_ = arg;
}

} // namespace commands
