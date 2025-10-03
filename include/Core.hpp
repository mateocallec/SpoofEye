/**
 * @file Core.hpp
 * @brief Main application core for SpoofEye: manages monitors and notifications.
 *
 * Licensed under GPLv3.
 */

#pragma once

#include "Config.hpp"
#include "monitors/ArpMonitor.hpp"
#include "monitors/DnsMonitor.hpp"
#include "monitors/IcmpMonitor.hpp"
#include "utils/Logger.hpp"
#include "utils/Notifier.hpp"

#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <string>

/**
 * @class Core
 * @brief Manages the lifecycle of network monitors (ARP, DNS, ICMP) and notifications.
 */
class Core {
public:
    /**
     * @brief Construct the core system.
     * @param pollIntervalSeconds Poll interval for monitors in seconds.
     * @param forcedGateway Optional forced gateway IP.
     * @param cfg Reference to application configuration.
     */
    Core(int pollIntervalSeconds, const std::string& forcedGateway, const Config& cfg);

    /**
     * @brief Run all enabled monitors until keepRunning is set to false.
     * @param keepRunning Atomic flag to control shutdown.
     */
    void run(std::atomic<bool>& keepRunning);

private:
    int m_pollIntervalSeconds;
    std::string m_forcedGateway;
    bool m_notificationsEnabled;

    std::optional<monitors::ArpMonitor> m_arpMonitor;
    std::optional<monitors::DnsMonitor> m_dnsMonitor;
    std::optional<monitors::IcmpMonitor> m_icmpMonitor;

    // ICMP alert throttling
    static constexpr std::chrono::seconds ICMP_ALERT_INTERVAL{60};
    std::mutex m_icmpMutex;
    std::chrono::steady_clock::time_point m_lastIcmpAlert;
};
