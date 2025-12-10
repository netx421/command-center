#!/usr/bin/env bash
set -e

echo "[command-center] Starting install..."

# ───────────────────────────────────────────────
# 1. Arch deps (optional but helpful for users)
# ───────────────────────────────────────────────
if [ -f /etc/os-release ] && grep -qi "arch" /etc/os-release; then
    echo "[command-center] Detected Arch-based system."
    echo "[command-center] Installing dependencies (requires sudo)..."
    sudo pacman -S --needed --noconfirm \
        base-devel \
        cmake \
        ninja \
        gtk3
else
    echo "[command-center] Non-Arch system detected."
    echo "Please ensure the following are installed manually, then re-run install.sh:"
    echo "  • base-devel (or build tools)"
    echo "  • cmake"
    echo "  • ninja"
    echo "  • gtk3 / gtk+-3.0 dev package"
fi

# ───────────────────────────────────────────────
# 2. Configure, build, install to ~/.local
# ───────────────────────────────────────────────
PREFIX="$HOME/.local"
BUILD_DIR="build"

echo "[command-center] Using install prefix: $PREFIX"
mkdir -p "$BUILD_DIR"

echo "[command-center] Configuring with CMake..."
cmake -S . -B "$BUILD_DIR" -G Ninja -DCMAKE_INSTALL_PREFIX="$PREFIX"

echo "[command-center] Building..."
cmake --build "$BUILD_DIR"

echo "[command-center] Installing..."
cmake --install "$BUILD_DIR"

# ───────────────────────────────────────────────
# 3. Refresh desktop database
# ───────────────────────────────────────────────
APPS_DIR="$PREFIX/share/applications"

if command -v update-desktop-database >/dev/null 2>&1; then
    echo "[command-center] Updating desktop database..."
    update-desktop-database "$APPS_DIR" || true
fi

echo
echo "[command-center] Install complete!"
echo "You should now see 'Command Center' in your app launcher (drun/wofi/etc)."
echo

