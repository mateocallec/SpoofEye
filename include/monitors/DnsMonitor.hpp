/**
 * @file DnsMonitor.hpp
 * @brief Monitors system DNS servers and alerts on unknown entries.
 * 
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

namespace monitors {

/**
 * @brief Default polling interval in seconds.
 */
inline constexpr std::chrono::seconds DEFAULT_POLL_INTERVAL{5};

/**
 * @brief Callback type for DNS change notifications.
 * @param title Notification title.
 * @param body Notification message body.
 */
using NotificationCallback = std::function<void(const std::string& title, const std::string& body)>;

/**
 * @class DnsMonitor
 * @brief Monitors system DNS servers and notifies when unknown DNS servers appear.
 */
class DnsMonitor {
public:
    // -------------------- Constructors / Destructor --------------------
    DnsMonitor();
    explicit DnsMonitor(std::chrono::seconds pollInterval);
    DnsMonitor(std::chrono::seconds pollInterval, const std::string& knownDnsPath);
    ~DnsMonitor();

    // Non-copyable
    DnsMonitor(const DnsMonitor&) = delete;
    DnsMonitor& operator=(const DnsMonitor&) = delete;

    // -------------------- Control --------------------
    void start();
    void stop();

    /**
     * @brief Set a custom notification callback.
     * @param cb Callback function to handle notifications.
     */
    void setNotificationCallback(NotificationCallback cb);

    /**
     * @brief Set or change the path to the known DNS JSON file.
     * @param path JSON file path.
     */
    void setKnownDnsPath(const std::string& path);

    /**
     * @brief Get the current known DNS JSON path.
     * @return Path to known DNS file.
     */
    std::string getKnownDnsPath() const;

    /**
     * @brief Force reload of the known DNS JSON file.
     * @return True if reload succeeded.
     */
    bool reloadKnownDns();

    /**
     * @brief Get a thread-safe snapshot of last observed DNS servers.
     */
    std::vector<std::string> getLastObservedDns() const;

    /**
     * @brief Check if monitor is currently alerting (unknown DNS detected).
     */
    bool isAlerting() const;

    /**
     * @brief Check if the monitor is initialized (has known DNS loaded).
     */
    bool isInitialized() const;

private:
    // -------------------- Internal Worker --------------------
    void workerLoop();
    void checkOnce();
    std::vector<std::string> getSystemDnsServers() const;
    void notify(const std::string& title, const std::string& body);
    bool loadKnownDnsFromFile(const std::string& path);

    // -------------------- Members --------------------
    std::chrono::seconds m_pollInterval;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_alerting{false};
    NotificationCallback m_notifyCb;

    mutable std::recursive_mutex m_mutex;
    std::set<std::string> m_knownDns;           ///< Known DNS servers
    std::vector<std::string> m_lastObservedDns; ///< Last observed DNS snapshot
    std::set<std::string> m_lastUnknownDns;     ///< Last detected unknown DNS

    std::thread m_thread;
    std::string m_knownDnsPath; ///< Path to known DNS JSON
    bool stopped_ = false;
};

} // namespace monitors
