/**
 * @file Version.hpp
 * @brief Defines the Version command for SpoofEye.
 * 
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#pragma once

#include "commands/Init.hpp"

namespace commands {

/**
 * @class Version
 * @brief Command to display the software version and related information.
 */
class Version : public Command {
public:
    /**
     * @brief Constructor.
     */
    Version();

    /**
     * @brief Executes the version command.
     * @param arg Optional argument (ignored).
     * 
     * Prints software name, version, author, license, copyright,
     * repository URL, and description.
     */
    void execute(const std::string& arg = "") override;
};

} // namespace commands
