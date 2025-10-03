/**
 * @file Version.cpp
 * @brief Implementation of the Version command for SpoofEye.
 *
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "commands/Version.hpp"
#include "utils/Logger.hpp"
#include "constants.hpp"

#include <string>

namespace commands {

Version::Version() = default;

void Version::execute(const std::string&) {
    // Print full software information
    Logger::print(
        std::string(SOFTWARE_NAME) + " v" + std::string(SOFTWARE_VERSION) + "\n"
        + "Author: " + std::string(SOFTWARE_AUTHOR) + "\n"
        + "License: " + std::string(SOFTWARE_LICENSE) + "\n"
        + "Copyright (c) " + std::string(SOFTWARE_COPYRIGHT_DATE) + "\n\n"
        + "Github: " + std::string(SOFTWARE_REPOSITORY) + "\n"
        + "Description: " + std::string(SOFTWARE_DESCRIPTION) + "\n\n"
        + "This program is free software: you can redistribute it and/or modify" + "\n"
        + "it under the terms of the GNU General Public License as published by" + "\n"
        + "the Free Software Foundation, either version 3 of the License, or" + "\n"
        + "(at your option) any later version." + "\n\n"
        + "This program is distributed in the hope that it will be useful," + "\n"
        + "but WITHOUT ANY WARRANTY; without even the implied warranty of" + "\n"
        + "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" + "\n"
        + "GNU General Public License for more details." + "\n\n"
        + "You should have received a copy of the GNU General Public License" + "\n"
        + "along with this program.  If not, see <https://www.gnu.org/licenses/>."
    );
}

} // namespace commands
