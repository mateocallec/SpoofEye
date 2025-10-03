#!/usr/bin/env bash
# tests/emulate_arp_spoofing.sh
# Emulates ARP spoofing by temporarily replacing the MAC address of a target IP in the neighbor table.
# This script is intended for testing and educational purposes only.
# Unauthorized use on networks without permission is illegal and unethical.
# This script can damage your operating system!

set -euo pipefail

SCRIPT_NAME="$(basename "$0")"
DEFAULT_IFACE="wlan0"
DEFAULT_MAC_TEMP="11:22:33:44:55:66"
mac_regex='^([0-9A-Fa-f]{2}(:|-)){5}[0-9A-Fa-f]{2}$'

print_usage() {
  cat <<EOF
Usage: $SCRIPT_NAME [MAC_TEMP] [IFACE] [--reset]
Defaults:
  MAC_TEMP = $DEFAULT_MAC_TEMP
  IFACE    = $DEFAULT_IFACE
Actions:
  (no arguments)         -> Detects a relevant IP and replaces its MAC with MAC_TEMP
  --reset [IFACE]        -> Removes ip neigh entries where lladdr == MAC_TEMP on IFACE
  [MAC_TEMP] [IFACE]     -> Uses the provided MAC and/or IFACE
EOF
}

# Defaults
IFACE="$DEFAULT_IFACE"
MAC_TEMP="$DEFAULT_MAC_TEMP"
ACTION="apply"  # apply | reset

# Parse arguments (order-independent)
while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help) print_usage; exit 0;;
    --reset) ACTION="reset"; shift;;
    --apply) ACTION="apply"; shift;;
    *)
      if [[ "$1" =~ $mac_regex ]]; then
        # Normalize MAC to lowercase and colons
        m="${1//-/:}"
        MAC_TEMP="${m,,}"
      else
        IFACE="$1"
      fi
      shift
      ;;
  esac
done

# Helper function to check if a command exists
cmd_exists() { command -v "$1" >/dev/null 2>&1; }

# Strategies to find an IP
find_ip_from_proc_arp() {
  local iface="$1"
  if [[ -r /proc/net/arp ]]; then
    awk -v dev="$iface" 'NR>1 && $6==dev { print $1; exit }' /proc/net/arp || true
  fi
}

find_ip_from_ip_neigh() {
  local iface="$1"
  if cmd_exists ip; then
    # Prefer lines with lladdr and a state
    ip neigh show dev "$iface" 2>/dev/null | awk '
      /lladdr/ && /REACHABLE|STALE|DELAY|PROBE/ { print $1; exit }
      /lladdr/ { if (!found) { first=$1; found=1 } }
      END { if (found) print first }
    ' || true
  fi
}

find_gateway_ip() {
  local iface="$1"
  if cmd_exists ip; then
    ip route show dev "$iface" 2>/dev/null | awk '/default/ { for(i=1;i<=NF;i++) if($i=="via"){ print $(i+1); exit } }' || true
  fi
}

find_ips_with_mac_temp() {
  local iface="$1" mac="$2"
  if cmd_exists ip; then
    ip neigh show dev "$iface" 2>/dev/null | awk -v mac="$mac" 'BEGIN{IGNORECASE=1} $0 ~ mac { print $1 }' || true
  else
    if [[ -r /proc/net/arp ]]; then
      awk -v dev="$iface" -v mac="$mac" 'NR>1 && tolower($4)==tolower(mac) && $6==dev { print $1 }' /proc/net/arp || true
    fi
  fi
}

# Main logic
if [[ "$ACTION" == "reset" ]]; then
  echo "[*] Reset requested — searching for entries with lladdr=$MAC_TEMP on $IFACE..."
  IPS="$(find_ips_with_mac_temp "$IFACE" "$MAC_TEMP" || true)"
  if [[ -z "${IPS:-}" ]]; then
    echo "No entries found with MAC $MAC_TEMP on $IFACE."
    exit 0
  fi
  while IFS= read -r ipaddr; do
    [[ -z "$ipaddr" ]] && continue
    echo "Deleting: sudo ip neigh del $ipaddr dev $IFACE"
    if sudo ip neigh del "$ipaddr" dev "$IFACE"; then
      echo "  -> Deleted: $ipaddr"
    else
      echo "  -> Failed to delete $ipaddr" >&2
    fi
  done <<< "$IPS"
  echo "Reset complete."
  exit 0
fi

# ACTION = apply
echo "[*] Apply mode — Temporary MAC: $MAC_TEMP ; Interface: $IFACE"
IP_FOUND=""
IP_FOUND="$(find_ip_from_proc_arp "$IFACE" || true)"
if [[ -z "$IP_FOUND" ]]; then
  IP_FOUND="$(find_ip_from_ip_neigh "$IFACE" || true)"
fi
if [[ -z "$IP_FOUND" ]]; then
  IP_FOUND="$(find_gateway_ip "$IFACE" || true)"
fi
if [[ -z "$IP_FOUND" ]]; then
  echo "Error: Could not determine a relevant IP for interface $IFACE." >&2
  echo "Check if the interface exists and if there are ARP entries or iproute2 installed." >&2
  exit 2
fi
echo "Determined IP: $IP_FOUND (interface: $IFACE)."

# Get current MAC for that IP (best-effort)
CUR_MAC=""
if [[ -r /proc/net/arp ]]; then
  CUR_MAC="$(awk -v ip="$IP_FOUND" 'NR>1 && $1==ip { print $4; exit }' /proc/net/arp || true)"
fi
if [[ -z "$CUR_MAC" && $(cmd_exists ip && echo yes || echo no) == "yes" ]]; then
  CUR_MAC="$(ip neigh show dev "$IFACE" 2>/dev/null | awk -v tgt="$IP_FOUND" 'tolower($1)==tolower(tgt) && /lladdr/ { print $3; exit }' || true)"
fi
if [[ -n "${CUR_MAC:-}" ]]; then
  echo "Current MAC found for $IP_FOUND: $CUR_MAC"
else
  echo "No current MAC found for $IP_FOUND (entry may be INCOMPLETE)."
fi

echo "Executing: sudo ip neigh replace $IP_FOUND lladdr $MAC_TEMP dev $IFACE nud permanent"
if sudo ip neigh replace "$IP_FOUND" lladdr "$MAC_TEMP" dev "$IFACE" nud permanent; then
  echo "Replacement successful: $IP_FOUND -> lladdr $MAC_TEMP (temporary/permanent in neigh table)."
  echo "To undo, use: $SCRIPT_NAME --reset $IFACE"
  exit 0
else
  echo "The 'ip neigh replace' command failed." >&2
  exit 3
fi
