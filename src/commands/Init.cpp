/**
 * @file Init.cpp
 * @brief Implementation of command manager for SpoofEye.
 *
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "commands/Init.hpp"
#include "utils/Logger.hpp"

#include <sstream>

namespace commands {

Init::Init() = default;

void Init::registerCommand(const std::string& name,
                           std::unique_ptr<Command> cmd,
                           const std::string& description,
                           const std::vector<std::string>& aliases,
                           bool takesArgument,
                           bool exitAfterExecution) {
    // Store command in primary commands map
    CommandInfo info{std::move(cmd), description, aliases, takesArgument, exitAfterExecution};
    primaryCommands_[name] = std::move(info);

    // Add lookup entries for main name and aliases
    const CommandInfo* infoPtr = &primaryCommands_[name];
    commands_[name] = infoPtr;
    for (const auto& alias : aliases) {
        commands_[alias] = infoPtr;
    }
}

bool Init::executeCommand(const std::string& name, const std::string& arg) const {
    auto it = commands_.find(name);
    if (it != commands_.end() && it->second) {
        const CommandInfo* info = it->second;
        if (info->takesArgument) {
            info->impl->execute(arg);
        } else {
            info->impl->execute();
        }
        return true;
    }
    return false;
}

const CommandInfo* Init::getCommandInfo(const std::string& name) const {
    auto it = commands_.find(name);
    return (it != commands_.end()) ? it->second : nullptr;
}

void Init::printCommands() const {
    for (const auto& [name, info] : primaryCommands_) {
        std::ostringstream line;
        line << "  " << name;

        // Append aliases if any
        if (!info.aliases.empty()) {
            line << " (aliases: ";
            for (size_t i = 0; i < info.aliases.size(); ++i) {
                line << info.aliases[i];
                if (i + 1 < info.aliases.size()) line << ", ";
            }
            line << ")";
        }

        // Append description if present
        if (!info.description.empty()) {
            line << ": " << info.description;
        }

        // Append exit flag note
        if (info.exitAfterExecution) {
            line << " [stops execution]";
        }

        Logger::print(line.str(), Logger::LogType::INFO);
    }
}

} // namespace commands
