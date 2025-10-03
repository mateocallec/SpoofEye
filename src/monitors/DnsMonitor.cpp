/**
 * @file DnsMonitor.cpp
 * @brief Implementation of DnsMonitor for SpoofEye.
 *
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "monitors/DnsMonitor.hpp"
#include "monitors/Init.hpp"
#include "utils/Logger.hpp"

#include "lib/json.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <thread>

#ifdef _WIN32
#define NOMINMAX
#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

namespace monitors {

// -------------------- Utility --------------------
static std::string normalizeIpString(std::string s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
        [](unsigned char ch){ return !std::isspace(ch); }));
    s.erase(std::find_if(s.rbegin(), s.rend(),
        [](unsigned char ch){ return !std::isspace(ch); }).base(), s.end());
    return s;
}

// -------------------- Constructors --------------------
DnsMonitor::DnsMonitor()
    : m_pollInterval(DEFAULT_POLL_INTERVAL), m_running(false), m_alerting(false) {}

DnsMonitor::DnsMonitor(std::chrono::seconds pollInterval)
    : m_pollInterval(pollInterval), m_running(false), m_alerting(false) {}

DnsMonitor::DnsMonitor(std::chrono::seconds pollInterval, const std::string& knownDnsPath)
    : m_pollInterval(pollInterval), m_running(false), m_alerting(false) {
    loadKnownDnsFromFile(knownDnsPath);
}

DnsMonitor::~DnsMonitor() {
    stop();
}

// -------------------- Control --------------------
void DnsMonitor::start() {
    Logger::log("DNS monitor enabled", Logger::LogType::DEFAULT, LogPrefixes::dns_monitor);
    bool expected = false;
    if (!m_running.compare_exchange_strong(expected, true)) return;
    m_thread = std::thread(&DnsMonitor::workerLoop, this);
}

void DnsMonitor::stop() {
    if (stopped_) return;
    stopped_ = true;

    bool expected = true;
    if (m_running.compare_exchange_strong(expected, false)) {
        if (m_thread.joinable()) m_thread.join();
    }

    Logger::log("DNS monitor stopped", Logger::LogType::DEFAULT, LogPrefixes::dns_monitor);
}

void DnsMonitor::setNotificationCallback(NotificationCallback cb) {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    m_notifyCb = std::move(cb);
}

void DnsMonitor::setKnownDnsPath(const std::string& path) {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    loadKnownDnsFromFile(path);
}

std::string DnsMonitor::getKnownDnsPath() const {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    return m_knownDnsPath;
}

bool DnsMonitor::reloadKnownDns() {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    if (m_knownDnsPath.empty()) return false;
    return loadKnownDnsFromFile(m_knownDnsPath);
}

std::vector<std::string> DnsMonitor::getLastObservedDns() const {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    return m_lastObservedDns;
}

bool DnsMonitor::isAlerting() const {
    return m_alerting.load();
}

bool DnsMonitor::isInitialized() const {
    std::lock_guard<std::recursive_mutex> lk(m_mutex);
    return !m_knownDns.empty();
}

// -------------------- Worker --------------------
void DnsMonitor::workerLoop() {
    while (m_running.load()) {
        try {
            checkOnce();
        } catch (const std::exception& ex) {
            Logger::log("Exception in workerLoop: " + std::string(ex.what()), Logger::LogType::ERROR, LogPrefixes::dns_monitor);
        } catch (...) {
            Logger::log("Unknown exception in worker loop.", Logger::LogType::ERROR, LogPrefixes::dns_monitor);
        }
        std::this_thread::sleep_for(m_pollInterval);
    }
}

void DnsMonitor::checkOnce() {
    auto current = getSystemDnsServers();
    {
        std::lock_guard<std::recursive_mutex> lk(m_mutex);
        m_lastObservedDns = current;
    }

    std::set<std::string> currentSet(current.begin(), current.end());
    std::vector<std::string> unknowns;

    {
        std::lock_guard<std::recursive_mutex> lk(m_mutex);
        for (const auto& s : currentSet) {
            if (m_knownDns.find(s) == m_knownDns.end()) unknowns.push_back(s);
        }
    }

    std::set<std::string> currentUnknowns(unknowns.begin(), unknowns.end());
    bool notifyChange = false;
    {
        std::lock_guard<std::recursive_mutex> lk(m_mutex);
        if (currentUnknowns != m_lastUnknownDns) {
            m_lastUnknownDns = currentUnknowns;
            notifyChange = true;
        }
    }

    if (notifyChange && !currentUnknowns.empty()) {
        std::ostringstream body;
        body << "Unrecognized DNS server(s) detected: ";
        for (size_t i = 0; i < unknowns.size(); ++i) {
            body << unknowns[i];
            if (i != unknowns.size() - 1) body << ", ";
        }
        notify("Unknown DNS", body.str());
        m_alerting = true;
    } else if (currentUnknowns.empty()) {
        bool wasAlerting = m_alerting.exchange(false);
        if (wasAlerting) {
            notify("DNS Monitor — Resolved", "Previously detected unknown DNS servers are no longer present.");
        }
    }
}

// -------------------- System DNS --------------------
std::vector<std::string> DnsMonitor::getSystemDnsServers() const {
    std::vector<std::string> out;
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);

    ULONG flags = GAA_FLAG_SKIP_UNICAST | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;
    ULONG family = AF_UNSPEC;
    ULONG bufferSize = 0;
    GetAdaptersAddresses(family, flags, nullptr, nullptr, &bufferSize);
    std::vector<uint8_t> buffer(bufferSize);
    PIP_ADAPTER_ADDRESSES adapters = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
    DWORD res = GetAdaptersAddresses(family, flags, nullptr, adapters, &bufferSize);

    if (res == NO_ERROR) {
        for (PIP_ADAPTER_ADDRESSES cur = adapters; cur != nullptr; cur = cur->Next) {
            for (PIP_ADAPTER_DNS_SERVER_ADDRESS dns = cur->FirstDnsServerAddress; dns; dns = dns->Next) {
                SOCKADDR *sa = dns->Address.lpSockaddr;
                char addrStr[INET6_ADDRSTRLEN] = {0};
                if (!sa) continue;
                if (sa->sa_family == AF_INET) {
                    sockaddr_in *v4 = reinterpret_cast<sockaddr_in*>(sa);
                    inet_ntop(AF_INET, &v4->sin_addr, addrStr, sizeof(addrStr));
                    out.emplace_back(addrStr);
                } else if (sa->sa_family == AF_INET6) {
                    sockaddr_in6 *v6 = reinterpret_cast<sockaddr_in6*>(sa);
                    inet_ntop(AF_INET6, &v6->sin6_addr, addrStr, sizeof(addrStr));
                    out.emplace_back(addrStr);
                }
            }
        }
    } else {
        Logger::log("GetAdaptersAddresses failed: " + std::to_string(res), Logger::LogType::ERROR, LogPrefixes::dns_monitor);
    }
    WSACleanup();
#else
    std::ifstream ifs("/etc/resolv.conf");
    if (!ifs) {
        Logger::log("Unable to open /etc/resolv.conf", Logger::LogType::ERROR, LogPrefixes::dns_monitor);
        return out;
    }
    std::string line;
    while (std::getline(ifs, line)) {
        auto pos = line.find('#');
        if (pos != std::string::npos) line.erase(pos);
        std::istringstream iss(line);
        std::string token;
        if (!(iss >> token)) continue;
        if (token == "nameserver") {
            std::string addr;
            if (iss >> addr) out.push_back(normalizeIpString(addr));
        }
    }
#endif

    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

// -------------------- Notification --------------------
void DnsMonitor::notify(const std::string& title, const std::string& body) {
    NotificationCallback cbCopy;
    {
        std::lock_guard<std::recursive_mutex> lk(m_mutex);
        cbCopy = m_notifyCb;
    }

    if (cbCopy) {
        cbCopy(title, body);
        return;
    }

#if defined(__linux__) && !defined(__ANDROID__)
    std::ostringstream cmd;
    std::string t = title, b = body;
    std::replace(t.begin(), t.end(), '"', '\'');
    std::replace(b.begin(), b.end(), '"', '\'');
    cmd << "notify-send \"" << t << "\" \"" << b << "\"";
    if (std::system(cmd.str().c_str()) == -1)
        Logger::log(title + " - " + body, Logger::LogType::ERROR, LogPrefixes::dns_monitor);
#elif defined(__APPLE__)
    std::ostringstream cmd;
    std::string t = title, b = body;
    std::replace(t.begin(), t.end(), '"', '\'');
    std::replace(b.begin(), b.end(), '"', '\'');
    cmd << "osascript -e 'display notification \"" << b << "\" with title \"" << t << "\"'";
    if (std::system(cmd.str().c_str()) == -1)
        Logger::log(title + " - " + body, Logger::LogType::ERROR, LogPrefixes::dns_monitor);
#elif defined(_WIN32)
    std::string msg = title + "\n\n" + body;
    MessageBoxA(nullptr, msg.c_str(), "DNS Monitor", MB_OK | MB_ICONWARNING);
#else
    Logger::log(title + " - " + body, Logger::LogType::ERROR, LogPrefixes::dns_monitor);
#endif
}

// -------------------- Load Known DNS --------------------
bool DnsMonitor::loadKnownDnsFromFile(const std::string& path) {
    if (path.empty()) return false;

    std::ifstream ifs(path);
    if (!ifs) {
        Logger::log("Failed to open known DNS JSON: " + path, Logger::LogType::ERROR, LogPrefixes::dns_monitor);
        return false;
    }

    try {
        nlohmann::json j;
        ifs >> j;
        if (!j.is_array()) {
            Logger::log("Known DNS JSON is not an array.", Logger::LogType::ERROR, LogPrefixes::dns_monitor);
            return false;
        }

        std::set<std::string> tmp;
        for (auto &entry : j) {
            if (!entry.is_string()) {
                Logger::log("Known DNS JSON contains non-string entries.", Logger::LogType::ERROR, LogPrefixes::dns_monitor);
                return false;
            }
            std::string s = normalizeIpString(entry.get<std::string>());
            if (!s.empty()) tmp.insert(s);
        }

        if (tmp.empty()) {
            Logger::log("No valid DNS entries found in the JSON file.", Logger::LogType::ERROR, LogPrefixes::dns_monitor);
            return false;
        }

        std::lock_guard<std::recursive_mutex> lk(m_mutex);
        m_knownDns = std::move(tmp);
        m_knownDnsPath = path;

        return true;
    } catch (const std::exception& ex) {
        Logger::log("JSON parse error: " + std::string(ex.what()), Logger::LogType::ERROR, LogPrefixes::dns_monitor);
        return false;
    }
}

} // namespace monitors
