/**
 * @file PrintConfig.hpp
 * @brief Defines the PrintConfig command for SpoofEye.
 * 
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#pragma once

#include "commands/Init.hpp"
#include "Config.hpp"

#include <string>

namespace commands {

/**
 * @class PrintConfig
 * @brief Command to print the current software configuration.
 * 
 * Provides a command to trigger printing of configuration information.
 */
class PrintConfig : public Command {
public:
    /**
     * @brief Constructor.
     */
    explicit PrintConfig();

    /**
     * @brief Executes the command.
     * @param arg Optional argument (ignored).
     * 
     * Sets the internal flag indicating that configuration should be printed.
     */
    void execute(const std::string& arg = "") override;

    /**
     * @brief Prints the configuration to the log.
     * @param cfg Configuration object to print.
     */
    static void printConfiguration(const Config& cfg);

    /// Flag indicating whether the configuration should be printed
    static bool print_config;
};

} // namespace commands
