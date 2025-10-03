/**
 * @file ConfigPath.hpp
 * @brief Defines the ConfigPath command for SpoofEye.
 * 
 * This command allows setting the configuration file path.
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#pragma once

#include "commands/Init.hpp"

#include <string>

namespace commands {

/**
 * @class ConfigPath
 * @brief Command to set the configuration file path.
 * 
 * This command stores a reference to a string where the provided
 * configuration path will be saved. It validates that an argument
 * is provided before storing.
 */
class ConfigPath : public Command {
public:
    /**
     * @brief Constructor.
     * @param configPathRef Reference to a string that will hold the configuration path.
     */
    explicit ConfigPath(std::string& configPathRef);

    /**
     * @brief Executes the command.
     * @param arg The path to the configuration file. Must not be empty.
     * 
     * If no argument is provided, the program will log an error and exit.
     */
    void execute(const std::string& arg = "") override;

private:
    std::string& configPath_; ///< Reference to the configuration path string.
};

} // namespace commands
