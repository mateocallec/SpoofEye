/**
 * @file IcmpMonitor.cpp
 * @brief Implementation of IcmpMonitor for SpoofEye.
 *
 * Part of the SpoofEye project, licensed under GPLv3.
 */

#include "monitors/IcmpMonitor.hpp"
#include "monitors/Init.hpp"
#include "utils/Logger.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <pcap.h>

namespace monitors {

IcmpMonitor::IcmpMonitor()
    : m_pollInterval(std::chrono::milliseconds(100)), m_running(false), m_handle(nullptr) {}

IcmpMonitor::IcmpMonitor(std::chrono::milliseconds pollInterval)
    : m_pollInterval(pollInterval), m_running(false), m_handle(nullptr) {}

IcmpMonitor::~IcmpMonitor() {
    stop();
}

void IcmpMonitor::start() {
    bool expected = false;
    if (!m_running.compare_exchange_strong(expected, true)) return;
    m_thread = std::thread(&IcmpMonitor::workerLoop, this);
}

void IcmpMonitor::stop() {
    bool expected = true;
    if (m_running.compare_exchange_strong(expected, false)) {
        if (m_handle) {
            pcap_breakloop(m_handle); // Interrupt pcap_next_ex
        }
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }
}

void IcmpMonitor::setPingCallback(Callback cb) {
    m_callback = std::move(cb);
}

void IcmpMonitor::workerLoop() {
    char errbuf[PCAP_ERRBUF_SIZE];
    m_handle = pcap_open_live("any", 65536, 1, static_cast<int>(m_pollInterval.count()), errbuf);
    if (!m_handle) {
        Logger::log(std::string("pcap_open_live failed: ") + errbuf, Logger::LogType::ERROR, LogPrefixes::icmp_monitor);
        return;
    }

    struct bpf_program fp{};
    if (pcap_compile(m_handle, &fp, "icmp", 1, PCAP_NETMASK_UNKNOWN) == -1) {
        Logger::log(std::string("pcap_compile failed: ") + pcap_geterr(m_handle), Logger::LogType::ERROR, LogPrefixes::icmp_monitor);
        pcap_close(m_handle);
        return;
    }
    if (pcap_setfilter(m_handle, &fp) == -1) {
        Logger::log(std::string("pcap_setfilter failed: ") + pcap_geterr(m_handle), Logger::LogType::ERROR, LogPrefixes::icmp_monitor);
        pcap_close(m_handle);
        return;
    }

    Logger::log("ICMP monitor enabled", Logger::LogType::DEFAULT, LogPrefixes::icmp_monitor);

    while (m_running.load()) {
        struct pcap_pkthdr* header{};
        const u_char* packet{};
        int ret = pcap_next_ex(m_handle, &header, &packet);

        if (ret == 1 && packet) {
            int linkType = pcap_datalink(m_handle);
            const u_char* ipPacket = packet;

            // Adjust for link-layer header
            if (linkType == DLT_EN10MB) ipPacket += 14;
            else if (linkType == DLT_LINUX_SLL) ipPacket += 16;
            else if (linkType == DLT_NULL || linkType == DLT_LOOP) ipPacket += 4;
            else continue;

            auto* iphdr = reinterpret_cast<const struct ip*>(ipPacket);
            if (iphdr->ip_p != IPPROTO_ICMP) continue;

            auto* icmp = reinterpret_cast<const struct icmphdr*>(ipPacket + iphdr->ip_hl * 4);
            if (icmp->type == ICMP_ECHO) {
                std::string srcIp = inet_ntoa(iphdr->ip_src);
                auto now = std::chrono::steady_clock::now();

                if (m_callback && (now - m_lastNotification >= NOTIFY_INTERVAL)) {
                    m_callback(srcIp);
                    m_lastNotification = now;
                }
            }
        } else if (ret == -1) {
            Logger::log(std::string("pcap_next_ex error: ") + pcap_geterr(m_handle), Logger::LogType::ERROR, LogPrefixes::icmp_monitor);
        }
        // ret == 0: timeout, loop again
    }

    pcap_close(m_handle);
    m_handle = nullptr;
    Logger::log("ICMP monitor stopped", Logger::LogType::DEFAULT, LogPrefixes::icmp_monitor);
}

} // namespace monitors
