/**
 * @file Init.cpp
 * @brief Common definitions for network monitors in SpoofEye.
 * 
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "monitors/Init.hpp"

namespace monitors {

// Initialize log prefix constants
const std::string LogPrefixes::arp_monitor = "ARP Monitor";
const std::string LogPrefixes::dns_monitor = "DNS Monitor";
const std::string LogPrefixes::icmp_monitor = "ICMP Monitor";

} // namespace monitors
