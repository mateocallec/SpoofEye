/**
 * @file Config.hpp
 * @brief Simple INI-based configuration reader for SpoofEye.
 *
 * Supports basic section handling. Top-level keys include:
 *   - output_log_path
 *   - show_notifications
 *   - stylize_output
 *   - known_dns_path
 *
 * Section [Monitors] supports:
 *   - arp_monitor
 *   - dns_monitor
 *   - icmp_monitor
 *
 * Licensed under GPLv3.
 */

#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>

/**
 * @class Config
 * @brief Reads and provides access to SpoofEye INI configuration.
 */
class Config {
public:
    /**
     * @brief Construct a Config object and load the INI file.
     * @param ini_path Path to the configuration file.
     * @throws std::runtime_error if the file cannot be read.
     */
    explicit Config(const std::string& ini_path);

    /** @brief Reload the configuration file from disk. */
    void reload();

    /** @brief Get the path of the configuration file. */
    const std::string& getConfigPath() const noexcept;

    // ----- Top-level keys -----
    const std::string& getOutputLogPath() const noexcept;
    bool showNotifications() const noexcept;
    bool stylizeOutput() const noexcept;
    const std::string& getKnownDNSPath() const noexcept;

    // ----- Monitors section -----
    bool arpMonitorEnabled() const noexcept;
    bool dnsMonitorEnabled() const noexcept;
    bool icmpMonitorEnabled() const noexcept;

    // ----- Generic access -----
    bool hasKey(const std::string& key) const noexcept;
    std::string getRaw(const std::string& key) const noexcept;
    std::string summary() const;

private:
    std::string iniPath_;
    std::unordered_map<std::string, std::string> data_;

    void loadFromFile();
    static std::string trim(const std::string& s);
    static std::string toLower(const std::string& s);
    static bool parseBool(const std::string& s, bool defaultValue) noexcept;
};
