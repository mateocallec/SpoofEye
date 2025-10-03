/**
 * @file Init.hpp
 * @brief Common definitions for network monitors in SpoofEye.
 * 
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#pragma once

#include <string>

namespace monitors {

/**
 * @struct LogPrefixes
 * @brief Contains prefix strings used for logging different monitor types.
 */
struct LogPrefixes {
    /** Prefix for ARP monitor logs */
    static const std::string arp_monitor;

    /** Prefix for DNS monitor logs */
    static const std::string dns_monitor;

    /** Prefix for ICMP monitor logs */
    static const std::string icmp_monitor;
};

} // namespace monitors
