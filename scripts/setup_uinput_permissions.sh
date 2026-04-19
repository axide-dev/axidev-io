#!/usr/bin/env bash
set -euo pipefail

target_user="${SUDO_USER:-${USER:-}}"
if [ -z "$target_user" ] || [ "$target_user" = "root" ]; then
  echo "Run this script as a normal user; it will use sudo for the privileged steps." >&2
  exit 1
fi

rule_path="/etc/udev/rules.d/70-axidev-io-uinput.rules"
rule_contents='KERNEL=="uinput", MODE="0660", GROUP="input", OPTIONS+="static_node=uinput"'

sudo modprobe uinput
sudo groupadd -f input
sudo usermod -aG input "$target_user"
printf '%s\n' "$rule_contents" | sudo tee "$rule_path" >/dev/null
sudo udevadm control --reload-rules
sudo udevadm trigger /dev/uinput

cat <<EOF
Configured /dev/uinput access for ${target_user}.

Log out and back in before running the sender integration test so the new group
membership takes effect.
EOF
