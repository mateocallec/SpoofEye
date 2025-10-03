/**
 * @file Config.cpp
 * @brief Simple INI-based configuration reader for SpoofEye.
 *
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "Config.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

Config::Config(const std::string& ini_path)
    : iniPath_(ini_path)
{
    loadFromFile();
}

void Config::reload() {
    data_.clear();
    loadFromFile();
}

const std::string& Config::getConfigPath() const noexcept {
    return iniPath_;
}

// ----- Top-level keys -----
const std::string& Config::getOutputLogPath() const noexcept {
    static const std::string default_val = "";
    auto it = data_.find("output_log_path");
    return it != data_.end() ? it->second : default_val;
}

bool Config::showNotifications() const noexcept {
    auto it = data_.find("show_notifications");
    return it != data_.end() ? parseBool(it->second, true) : true;
}

bool Config::stylizeOutput() const noexcept {
    auto it = data_.find("stylize_output");
    return it != data_.end() ? parseBool(it->second, true) : true;
}

const std::string& Config::getKnownDNSPath() const noexcept {
    static const std::string default_val = "/etc/spoofeye/known_dns.json";
    auto it = data_.find("known_dns_path");
    return it != data_.end() ? it->second : default_val;
}

// ----- Monitors section -----
bool Config::arpMonitorEnabled() const noexcept {
    auto it = data_.find("monitors.arp_monitor");
    return it != data_.end() ? parseBool(it->second, true) : false;
}

bool Config::dnsMonitorEnabled() const noexcept {
    auto it = data_.find("monitors.dns_monitor");
    return it != data_.end() ? parseBool(it->second, true) : false;
}

bool Config::icmpMonitorEnabled() const noexcept {
    auto it = data_.find("monitors.icmp_monitor");
    return it != data_.end() ? parseBool(it->second, true) : false;
}

// ----- Generic access -----
bool Config::hasKey(const std::string& key) const noexcept {
    return data_.find(toLower(key)) != data_.end();
}

std::string Config::getRaw(const std::string& key) const noexcept {
    auto it = data_.find(toLower(key));
    return it != data_.end() ? it->second : std::string{};
}

std::string Config::summary() const {
    std::ostringstream oss;
    oss << "Config file path: " << iniPath_ << "\n\n"
        << "Config summary:\n"
        << " - output_log_path       = " << getOutputLogPath() << "\n"
        << " - show_notifications   = " << (showNotifications() ? "true" : "false") << "\n"
        << " - stylize_output       = " << (stylizeOutput() ? "true" : "false") << "\n"
        << " - known_dns_path       = " << getKnownDNSPath() << "\n"
        << " - monitors.arp_monitor = " << (arpMonitorEnabled() ? "true" : "false") << "\n"
        << " - monitors.dns_monitor = " << (dnsMonitorEnabled() ? "true" : "false") << "\n"
        << " - monitors.icmp_monitor = " << (icmpMonitorEnabled() ? "true" : "false") << "\n";
    return oss.str();
}

// ----- Private helpers -----
void Config::loadFromFile() {
    std::ifstream ifs(iniPath_);
    if (!ifs) throw std::runtime_error("Failed to open config file: " + iniPath_);

    std::string line;
    std::string currentSection;
    while (std::getline(ifs, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        auto commentPos = line.find_first_of(";#");
        if (commentPos != std::string::npos) line.erase(commentPos);

        line = trim(line);
        if (line.empty()) continue;

        if (line.size() >= 2 && line.front() == '[' && line.back() == ']') {
            currentSection = toLower(trim(line.substr(1, line.size() - 2)));
            continue;
        }

        std::size_t eq = line.find_first_of("=:");
        if (eq == std::string::npos) continue;

        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));
        if (!currentSection.empty()) key = currentSection + "." + key;
        data_[toLower(key)] = value;
    }
}

std::string Config::trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(static_cast<unsigned char>(*start))) ++start;
    auto end = s.end();
    do { --end; } while (std::distance(start, end) >= 0 && std::isspace(static_cast<unsigned char>(*end)));
    return std::string(start, end + 1);
}

std::string Config::toLower(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) out.push_back(static_cast<char>(std::tolower(c)));
    return out;
}

bool Config::parseBool(const std::string& s, bool defaultValue) noexcept {
    if (s.empty()) return defaultValue;
    std::string t = toLower(trim(s));
    if (t == "1" || t == "true" || t == "yes" || t == "on")  return true;
    if (t == "0" || t == "false" || t == "no" || t == "off") return false;
    return defaultValue;
}
