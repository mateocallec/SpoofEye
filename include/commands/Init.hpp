/**
 * @file Init.hpp
 * @brief Command manager for SpoofEye.
 * 
 * Provides registration, lookup, execution, and help printing for CLI commands.
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace commands {

/**
 * @brief Abstract base class for all commands.
 */
class Command {
public:
    virtual ~Command() = default;

    /**
     * @brief Execute the command.
     * @param arg Optional argument for the command.
     */
    virtual void execute(const std::string& arg = "") = 0;
};

/**
 * @struct CommandInfo
 * @brief Stores metadata and implementation for a command.
 */
struct CommandInfo {
    std::unique_ptr<Command> impl;      ///< Command implementation
    std::string description;            ///< Command description
    std::vector<std::string> aliases;   ///< List of command aliases
    bool takesArgument = false;         ///< True if command requires an argument
    bool exitAfterExecution = false;    ///< True if execution stops the program after running
};

/**
 * @class Init
 * @brief Manages registration, lookup, and execution of commands.
 */
class Init {
public:
    /**
     * @brief Default constructor.
     */
    Init();

    /**
     * @brief Registers a command with optional metadata.
     * @param name Primary name of the command.
     * @param cmd Unique pointer to the command implementation.
     * @param description Optional command description.
     * @param aliases Optional list of aliases for the command.
     * @param takesArgument True if the command expects an argument.
     * @param exitAfterExecution True if program should stop after executing this command.
     */
    void registerCommand(const std::string& name,
                         std::unique_ptr<Command> cmd,
                         const std::string& description = "",
                         const std::vector<std::string>& aliases = {},
                         bool takesArgument = false,
                         bool exitAfterExecution = true);

    /**
     * @brief Executes a command by name or alias.
     * @param name Name or alias of the command.
     * @param arg Optional argument to pass to the command.
     * @return True if a matching command was executed.
     */
    bool executeCommand(const std::string& name, const std::string& arg = "") const;

    /**
     * @brief Prints all registered primary commands for help.
     */
    void printCommands() const;

    /**
     * @brief Retrieves metadata for a command by name or alias.
     * @param name Name or alias of the command.
     * @return Pointer to CommandInfo or nullptr if not found.
     */
    const CommandInfo* getCommandInfo(const std::string& name) const;

private:
    std::unordered_map<std::string, CommandInfo> primaryCommands_; ///< Primary commands by name
    std::unordered_map<std::string, const CommandInfo*> commands_; ///< Lookup map including aliases
};

} // namespace commands
