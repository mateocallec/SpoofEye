/**
 * @file Core.cpp
 * @brief Main application core for SpoofEye: manages monitors and notifications.
 *
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "Core.hpp"
#include "monitors/Init.hpp"
#include "constants.hpp"

#include <optional>
#include <thread>

Core::Core(int pollIntervalSeconds, const std::string& forcedGateway, const Config& cfg)
    : m_pollIntervalSeconds(pollIntervalSeconds),
      m_forcedGateway(forcedGateway),
      m_notificationsEnabled(cfg.showNotifications()),
      m_lastIcmpAlert(std::chrono::steady_clock::now() - ICMP_ALERT_INTERVAL)
{
    // Initialize logging
    Logger::init(cfg.getOutputLogPath(), cfg.stylizeOutput());
    Logger::log("Starting " + std::string(SOFTWARE_NAME) + " v" + std::string(SOFTWARE_VERSION) + "...");
    Logger::log("License: " + std::string(SOFTWARE_LICENSE));
    Logger::log("Configuration file path: " + cfg.getConfigPath());
    Logger::log("Output log path: " + cfg.getOutputLogPath());
    Logger::log("Initializing monitors...");

    // ----- ARP Monitor -----
    if (cfg.arpMonitorEnabled()) {
        if (forcedGateway.empty()) {
            m_arpMonitor.emplace(pollIntervalSeconds);
        } else {
            m_arpMonitor.emplace(forcedGateway, pollIntervalSeconds);
        }
    }

    // ----- DNS Monitor -----
    if (cfg.dnsMonitorEnabled()) {
        m_dnsMonitor.emplace(std::chrono::seconds(pollIntervalSeconds));
        m_dnsMonitor->setKnownDnsPath(cfg.getKnownDNSPath());
        m_dnsMonitor->setNotificationCallback([this](const std::string& title, const std::string& body) {
            Logger::log(title + " -> " + body, Logger::LogType::WARNING, monitors::LogPrefixes::dns_monitor);
            Notifier notifier(m_notificationsEnabled);
            notifier.send(title, body, Notifier::Level::WARNING, "dialog-warning");
        });
    }

    // ----- ICMP Monitor -----
    if (cfg.icmpMonitorEnabled()) {
        m_icmpMonitor.emplace(std::chrono::milliseconds(100));
        m_icmpMonitor->setPingCallback([this](const std::string& srcIp) {
            std::lock_guard<std::mutex> lock(m_icmpMutex);
            auto now = std::chrono::steady_clock::now();
            if (now - m_lastIcmpAlert >= ICMP_ALERT_INTERVAL) {
                std::string msg = "Ping detected from " + srcIp;
                Logger::log(msg, Logger::LogType::DEFAULT, monitors::LogPrefixes::icmp_monitor);
                Notifier notifier(m_notificationsEnabled);
                notifier.send("ICMP Ping Alert", msg, Notifier::Level::INFO, "network-transmit-receive");
                m_lastIcmpAlert = now;
            }
        });
    }
}

void Core::run(std::atomic<bool>& keepRunning) {
    // ----- Log monitored resources -----
    if (m_arpMonitor && m_arpMonitor->isInitialized()) {
        std::string gw = m_arpMonitor->gateway_ip();
        if (gw.empty()) {
            Logger::log("Could not detect gateway IP.",
                        Logger::LogType::ERROR);
            return;
        }
        Logger::log("Monitoring gateway IP: " + gw +
                    " (poll interval " + std::to_string(m_pollIntervalSeconds) + "s).",
                    Logger::LogType::INFO);
    }

    if (m_dnsMonitor && m_dnsMonitor->isInitialized()) {
        Logger::log("Monitoring DNS servers (poll interval " +
                    std::to_string(m_pollIntervalSeconds) + "s).", Logger::LogType::INFO);
    }

    // ----- Start monitor threads -----
    std::thread tArp, tDns, tIcmp;

    if (m_arpMonitor && m_arpMonitor->isInitialized()) {
        tArp = std::thread([this] {
            m_arpMonitor->start([this](const std::string& old_mac, const std::string& new_mac, const std::string& ip) {
                std::string oldStr = old_mac.empty() ? "(unknown)" : old_mac;
                std::string newStr = new_mac.empty() ? "(unknown)" : new_mac;
                Logger::log("ARP change detected at " + ip + " (" + oldStr + " -> " + newStr + ")",
                            Logger::LogType::CRITICAL, monitors::LogPrefixes::arp_monitor);
                Notifier notifier(m_notificationsEnabled);
                notifier.send("ARP Alert", "Gateway MAC changed from " + oldStr + " to " + newStr,
                              Notifier::Level::CRITICAL, "dialog-warning");
            });
        });
    }

    if (m_dnsMonitor && m_dnsMonitor->isInitialized()) {
        tDns = std::thread([this] { m_dnsMonitor->start(); });
    }

    if (m_icmpMonitor && m_icmpMonitor->isInitialized()) {
        tIcmp = std::thread([this] { m_icmpMonitor->start(); });
    }

    // ----- Main loop -----
    while (keepRunning.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // ----- Shutdown -----
    Logger::log("Shutting down " + std::string(SOFTWARE_NAME) + " v" + std::string(SOFTWARE_VERSION) + "...");
    Logger::log("Stopping monitors...");

    if (m_arpMonitor) m_arpMonitor->stop();
    if (m_dnsMonitor) m_dnsMonitor->stop();
    if (m_icmpMonitor) m_icmpMonitor->stop();

    if (tArp.joinable()) tArp.join();
    if (tDns.joinable()) tDns.join();
    if (tIcmp.joinable()) tIcmp.join();

    Logger::log("Exited.");
}
