# Command Center

A simple GTK-based system control panel for Arch / Hyprland desktops.

Command Center provides quick access to:

- Basic system info (hostname, kernel, uptime)
- One-click **system update** (opens terminal with `sudo pacman -Syu`)
- **Reset sound** (restarts PipeWire/WirePlumber or PulseAudio)
- **Reset network** (restarts NetworkManager in a terminal)
- **Shutdown** and **Restart** with confirmation dialogs

Designed as a friendly “command center” for users who don’t want to dig
through the terminal for common maintenance tasks.

---

## Dependencies (Arch Linux)

```bash
sudo pacman -S --needed base-devel cmake ninja gtk3
