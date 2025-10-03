/**
 * @file ArpMonitor.cpp
 * @brief Implementation of ARP monitor for gateway MAC address changes.
 *
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "monitors/ArpMonitor.hpp"
#include "monitors/Init.hpp"
#include "utils/Logger.hpp"

#include <atomic>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

using namespace std::chrono_literals;

namespace monitors {

namespace {

/**
 * @brief Execute a shell command and capture output.
 * @param cmd Command to execute.
 * @return Captured stdout with trailing newlines removed.
 */
std::string run_cmd_capture(const char* cmd) {
    std::string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return result;

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);

    // Trim trailing whitespace/newlines
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r')) {
        result.pop_back();
    }
    return result;
}

} // namespace

// Implementation details hidden in Pimpl
struct ArpMonitor::Impl {
    std::string gateway;
    int interval_seconds = 5;
    std::atomic<bool> running{false};
    std::mutex mtx;

    Impl() = default;
    ~Impl() = default;

    /**
     * @brief Detect the system's default gateway IP.
     * @return Detected gateway IP or empty string if not found.
     */
    static std::string detect_gateway_ip() {
        std::string out = run_cmd_capture("ip route show default 2>/dev/null");
        if (!out.empty()) {
            std::istringstream iss(out);
            std::string token;
            while (iss >> token) {
                if (token == "via" && (iss >> token)) return token;
            }
        }

        // Fallback using `route -n`
        out = run_cmd_capture("route -n 2>/dev/null");
        std::istringstream rss(out);
        std::string line;
        while (std::getline(rss, line)) {
            std::istringstream ls(line);
            std::string dest, gw;
            if (ls >> dest >> gw && dest == "0.0.0.0") return gw;
        }
        return {};
    }

    /**
     * @brief Read MAC address of given IP from /proc/net/arp.
     * @param ip IP address to lookup.
     * @return MAC address in lowercase, or empty if not found.
     */
    static std::string read_mac_from_proc_arp(const std::string& ip) {
        std::ifstream ifs("/proc/net/arp");
        if (!ifs.is_open()) return {};

        std::string line;
        std::getline(ifs, line); // skip header
        while (std::getline(ifs, line)) {
            std::istringstream iss(line);
            std::string ipaddr, hwtype, flags, hwaddr, mask, device;
            if (!(iss >> ipaddr >> hwtype >> flags >> hwaddr >> mask >> device)) continue;
            if (ipaddr == ip) {
                if (hwaddr == "00:00:00:00:00:00") return {};
                for (auto& c : hwaddr) c = std::tolower(static_cast<unsigned char>(c));
                return hwaddr;
            }
        }
        return {};
    }

    /**
     * @brief Normalize MAC address string (remove whitespace, lowercase).
     */
    static std::string normalize_mac(const std::string& mac) {
        std::string out;
        for (char c : mac) {
            if (!std::isspace(static_cast<unsigned char>(c))) {
                out.push_back(std::tolower(static_cast<unsigned char>(c)));
            }
        }
        return out;
    }

    /**
     * @brief Monitor loop to detect MAC changes.
     */
    void monitor_loop(ChangeCallback cb) {
        std::string last_mac = read_mac_from_proc_arp(gateway);
        if (!last_mac.empty()) {
            Logger::log("Initial MAC for gateway " + gateway + " : " + last_mac,
                        Logger::LogType::INFO, LogPrefixes::arp_monitor);
        } else {
            Logger::log("No ARP entry for gateway " + gateway + " (yet)",
                        Logger::LogType::INFO, LogPrefixes::arp_monitor);
        }

        while (running.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(interval_seconds));
            if (!running.load()) break;

            std::string current = normalize_mac(read_mac_from_proc_arp(gateway));
            std::string prev = normalize_mac(last_mac);

            if (current.empty() && prev.empty()) continue;

            if (current != prev) {
                if (prev.empty()) {
                    Logger::log("ARP entry appeared for gateway " + gateway + " : " + current,
                                Logger::LogType::INFO, LogPrefixes::arp_monitor);
                } else if (current.empty()) {
                    Logger::log("ARP entry for gateway " + gateway + " disappeared (was " + prev + ")",
                                Logger::LogType::CRITICAL, LogPrefixes::arp_monitor);
                } else {
                    Logger::log("MAC change for gateway " + gateway + " : " + prev + " -> " + current,
                                Logger::LogType::CRITICAL, LogPrefixes::arp_monitor);
                }

                if (cb) {
                    try { cb(prev, current, gateway); } catch (...) {}
                }
                last_mac = current;
            }
        }
    }
};

// ---------- Public Interface ----------

ArpMonitor::ArpMonitor(int poll_interval_seconds) {
    pimpl = std::make_unique<Impl>();
    pimpl->interval_seconds = (poll_interval_seconds > 0 ? poll_interval_seconds : 5);
    pimpl->gateway = Impl::detect_gateway_ip();
}

ArpMonitor::ArpMonitor(const std::string& gateway_ip, int poll_interval_seconds) {
    pimpl = std::make_unique<Impl>();
    pimpl->interval_seconds = (poll_interval_seconds > 0 ? poll_interval_seconds : 5);
    pimpl->gateway = gateway_ip.empty() ? Impl::detect_gateway_ip() : gateway_ip;
}

ArpMonitor::~ArpMonitor() {
    stop();
    pimpl.reset();
}

void ArpMonitor::start(ChangeCallback cb) {
    Logger::log("ARP monitor enabled", Logger::LogType::DEFAULT, LogPrefixes::arp_monitor);

    if (!pimpl) return;

    if (pimpl->gateway.empty()) {
        Logger::log("ERROR: Could not detect gateway IP. Exiting.",
                    Logger::LogType::ERROR, LogPrefixes::arp_monitor);
        return;
    }

    bool expected = false;
    if (!pimpl->running.compare_exchange_strong(expected, true)) {
        Logger::log("Monitor already running.", Logger::LogType::WARNING, LogPrefixes::arp_monitor);
        return;
    }

    pimpl->monitor_loop(cb);
}

void ArpMonitor::stop() {
    if (stopped_) return;
    stopped_ = true;

    if (!pimpl) return;

    bool expected = true;
    if (pimpl->running.compare_exchange_strong(expected, false)) {
        std::this_thread::sleep_for(100ms);
    }

    Logger::log("ARP monitor stopped", Logger::LogType::DEFAULT, LogPrefixes::arp_monitor);
}

std::string ArpMonitor::gateway_ip() const {
    return pimpl ? pimpl->gateway : std::string{};
}

} // namespace monitors
