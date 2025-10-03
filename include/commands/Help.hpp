/**
 * @file Help.hpp
 * @brief Defines the Help command for SpoofEye.
 * 
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#pragma once

#include "commands/Init.hpp"

namespace commands {

/**
 * @class Help
 * @brief Command to display help information about the software and available commands.
 * 
 * This command prints the software details, usage instructions,
 * a list of available commands, and example usages.
 */
class Help : public Command {
public:
    /**
     * @brief Constructor.
     * @param init Reference to the command manager (Init) to access all available commands.
     */
    explicit Help(const Init& init);

    /**
     * @brief Executes the help command.
     * @param arg Optional argument (ignored for help).
     * 
     * Prints software information, usage, available commands, and examples.
     */
    void execute(const std::string& arg = "") override;

private:
    const Init& cmdManager_; ///< Reference to the command manager to list all commands.
};

} // namespace commands
