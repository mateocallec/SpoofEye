/**
 * @file main.cpp
 * @brief Entry point for SpoofEye software.
 * 
 * Handles command-line arguments, initializes configuration, and runs the core application loop.
 * 
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "Core.hpp"
#include "Config.hpp"
#include "commands/Init.hpp"
#include "commands/ConfigPath.hpp"
#include "commands/Help.hpp"
#include "commands/PrintConfig.hpp"
#include "commands/Version.hpp"
#include "constants.hpp"

#include <atomic>
#include <csignal>
#include <iostream>
#include <string>
#include <vector>

/// Global flag to control the main loop termination
static std::atomic<bool> keepRunning{true};

/**
 * @brief Signal handler for SIGINT (Ctrl+C)
 * 
 * Sets the global keepRunning flag to false to stop the main loop gracefully.
 * 
 * @param signal Signal number (ignored)
 */
void handleSigint(int) {
    std::cout << std::endl;
    keepRunning.store(false);
}

/**
 * @brief Main entry point of SpoofEye
 * 
 * Parses command-line arguments, executes commands, initializes configuration, 
 * and runs the core loop until interrupted.
 * 
 * @param argc Number of command-line arguments
 * @param argv Array of command-line arguments
 * @return int Exit code (0 = success, non-zero = error)
 */
int main(int argc, char** argv) {
    int interval = 5;  ///< Default scan interval in seconds
    std::string forcedGateway;  ///< Optional forced gateway
    std::string iniPath = "/etc/spoofeye/spoofeye.ini";  ///< Default configuration file path

    // Initialize command manager
    commands::Init cmdManager;

    cmdManager.registerCommand(
        "--config-path",
        std::make_unique<commands::ConfigPath>(iniPath),
        "Override configuration file path",
        {},
        /*takesArgument=*/true,
        /*exitAfterExecution=*/false
    );

    cmdManager.registerCommand(
        "--help",
        std::make_unique<commands::Help>(cmdManager),
        "Show this help message",
        {"-h", "help"},
        /*takesArgument=*/false,
        /*exitAfterExecution=*/true
    );

    cmdManager.registerCommand(
        "--print-config",
        std::make_unique<commands::PrintConfig>(),
        "Print software configuration",
        {"--config", "config"},
        /*takesArgument=*/false,
        /*exitAfterExecution=*/false
    );

    // Register command-line commands
    cmdManager.registerCommand(
        "--version",
        std::make_unique<commands::Version>(),
        "Print software version",
        {"-v"},
        /*takesArgument=*/false,
        /*exitAfterExecution=*/true
    );

    // Convert argv to vector<string> for easier handling
    std::vector<std::string> args(argv + 1, argv + argc);

    // Execute registered commands first
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& token = args[i];
        const commands::CommandInfo* cmdInfo = cmdManager.getCommandInfo(token);

        if (!cmdInfo) {
            Logger::print(
                "Error: unknown command '" + token + "'\nRun '" + SOFTWARE_COMMAND + " --help' to see available commands.",
                Logger::LogType::ERROR
            );
            return 1;
        }

        std::string arg;
        if (cmdInfo->takesArgument) {
            if (i + 1 < args.size()) {
                arg = args[++i];  // Consume next token as argument
            } else {
                Logger::print("Error: command '" + token + "' requires an argument.", Logger::LogType::ERROR);
                return 2;
            }
        }

        cmdInfo->impl->execute(arg);

        if (cmdInfo->exitAfterExecution) {
            return 0;  // Exit if command indicates to stop
        }
    }

    // Parse standard flags for interval and gateway
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& token = args[i];

        if ((token == "--interval" || token == "-i") && i + 1 < args.size()) {
            interval = std::stoi(args[++i]);
        } else if (token.rfind("--interval=", 0) == 0) {
            interval = std::stoi(token.substr(11));
        } else if ((token == "--gateway" || token == "-g") && i + 1 < args.size()) {
            forcedGateway = args[++i];
        } else if (token.rfind("--gateway=", 0) == 0) {
            forcedGateway = token.substr(10);
        }
    }

    // Setup signal handling for graceful termination
    std::signal(SIGINT, handleSigint);

    try {
        // Load configuration
        Config cfg(iniPath);

        if (commands::PrintConfig::print_config) {
            commands::PrintConfig::printConfiguration(cfg);
        } else {
            Core core(interval, forcedGateway, cfg);
            core.run(keepRunning);
        }
    } catch (const std::exception& e) {
        Logger::print("Error: " + std::string(e.what()), Logger::LogType::ERROR);
        return 1;
    }

    return 0;
}
