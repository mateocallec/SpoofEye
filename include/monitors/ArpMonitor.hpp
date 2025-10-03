/**
 * @file ArpMonitor.hpp
 * @brief ARP monitor for detecting gateway MAC address changes.
 * 
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

namespace monitors {

/**
 * @class ArpMonitor
 * @brief Monitors the ARP entry of the network gateway for MAC changes.
 * 
 * Provides blocking monitoring with callback support for MAC address changes.
 */
class ArpMonitor {
public:
    /**
     * @brief Type of callback invoked on MAC change.
     * @param old_mac Previous MAC address (empty if none).
     * @param new_mac Current MAC address (empty if disappeared).
     * @param ip IP address of the gateway.
     */
    using ChangeCallback = std::function<void(const std::string& old_mac,
                                              const std::string& new_mac,
                                              const std::string& ip)>;

    /**
     * @brief Construct monitor using autodetected gateway IP.
     * @param poll_interval_seconds Polling interval in seconds (default 5).
     */
    explicit ArpMonitor(int poll_interval_seconds = 5);

    /**
     * @brief Construct monitor for a specific gateway IP.
     * @param gateway_ip Gateway IP address to monitor.
     * @param poll_interval_seconds Polling interval in seconds (default 5).
     */
    ArpMonitor(const std::string& gateway_ip, int poll_interval_seconds = 5);

    /**
     * @brief Destructor. Stops the monitor if running.
     */
    ~ArpMonitor();

    /**
     * @brief Start monitoring (blocking).
     * @param cb Optional callback to invoke on MAC changes.
     */
    void start(ChangeCallback cb = nullptr);

    /**
     * @brief Stop monitoring (thread-safe).
     */
    void stop();

    /**
     * @brief Get the detected gateway IP.
     * @return Gateway IP or empty string if not found.
     */
    std::string gateway_ip() const;

    /**
     * @brief Check if the monitor was successfully initialized.
     * @return True if a gateway IP is set.
     */
    bool isInitialized() const {
        return !gateway_ip().empty();
    }

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl; ///< Pimpl to hide implementation details
    bool stopped_ = false;       ///< Flag indicating if monitor was stopped
};

} // namespace monitors
