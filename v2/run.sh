#!/bin/bash
# Recompila e executa o programa
# Usage: bash run.sh

set -e
cd "$(dirname "$0")"

echo "=== Compilando ==="
mkdir -p build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release 2>&1 | tail -5
mingw32-make -j$(nproc) 2>&1 | tail -10
cd ..

echo ""
echo "=== Executando ==="
./build/extrator-movimento.exe
