/**
 * @file IcmpMonitor.hpp
 * @brief Monitors ICMP echo requests on the network and notifies via callback.
 * 
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <pcap.h>
#include <string>
#include <thread>

namespace monitors {

/**
 * @class IcmpMonitor
 * @brief Captures ICMP echo (ping) packets on all interfaces.
 */
class IcmpMonitor {
public:
    /** Callback type for ICMP echo notification */
    using Callback = std::function<void(const std::string& srcIp)>;

    /** Minimum interval between notifications for the same source */
    static constexpr std::chrono::seconds NOTIFY_INTERVAL{60};

    /** Default constructor with 100ms poll interval */
    IcmpMonitor();

    /** Constructor with custom poll interval */
    explicit IcmpMonitor(std::chrono::milliseconds pollInterval);

    /** Destructor stops monitoring if running */
    ~IcmpMonitor();

    /** Start monitoring in a background thread */
    void start();

    /** Stop monitoring and join thread */
    void stop();

    /** Set callback to be called when an ICMP echo is detected */
    void setPingCallback(Callback cb);

    /** Always returns true (monitor is always initialized) */
    bool isInitialized() const {
        return true;
    }

private:
    /** Background worker that captures ICMP packets */
    void workerLoop();

    std::chrono::milliseconds m_pollInterval;
    std::atomic<bool> m_running{false};
    std::thread m_thread;
    Callback m_callback{nullptr};
    pcap_t* m_handle{nullptr};
    std::chrono::steady_clock::time_point m_lastNotification{};
};

} // namespace monitors
