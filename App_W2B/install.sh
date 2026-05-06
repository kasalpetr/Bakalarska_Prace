#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=== DigitalBoard – instalace závislostí ==="

# System packages
if command -v apt-get &>/dev/null; then
    sudo apt-get update -qq
    sudo apt-get install -y cmake g++ libopencv-dev python3 python3-pip python3-tk
elif command -v dnf &>/dev/null; then
    sudo dnf install -y cmake gcc-c++ opencv-devel python3 python3-pip python3-tkinter
elif command -v pacman &>/dev/null; then
    sudo pacman -Sy --noconfirm cmake gcc opencv python python-pip tk
else
    echo "Nepodporovaný správce balíčků. Nainstaluj ručně: cmake, g++, libopencv-dev, python3, python3-pip, python3-tk"
    exit 1
fi

# Python packages
echo "=== Instalace Python závislostí ==="
pip3 install --user -r requirements.txt

# Build
echo "=== Sestavení aplikace ==="
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j"$(nproc)"
cd ..

echo ""
echo "=== Hotovo! ==="
echo "Spusť aplikaci příkazem: ./build/DigitalBoard"
