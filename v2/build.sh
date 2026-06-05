#!/bin/bash
# Build script for MSYS2 UCRT64
# Usage: bash build.sh

set -e
cd "$(dirname "$0")"

echo "=== Compilando ==="
mkdir -p build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
mingw32-make -j$(nproc)
cd ..

echo ""
echo "=== Coletando DLLs ==="
OUT_DIR="deploy"
rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

# Copiar executavel
cp build/extrator-movimento.exe "$OUT_DIR/"

DLL_DIR="/ucrt64/bin"

# Qt DLLs principais
for dll in Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll; do
    cp "$DLL_DIR/$dll" "$OUT_DIR/"
    echo "Copiado: $dll"
done

# Plugin de plataforma Qt (obrigatorio)
mkdir -p "$OUT_DIR/platforms"
cp "$DLL_DIR/../share/qt5/plugins/platforms/qwindows.dll" "$OUT_DIR/platforms/"
echo "Copiado: platforms/qwindows.dll"

# Plugin de estilos Qt
mkdir -p "$OUT_DIR/styles"
if [ -f "$DLL_DIR/../share/qt5/plugins/styles/qwindowsvistastyle.dll" ]; then
    cp "$DLL_DIR/../share/qt5/plugins/styles/qwindowsvistastyle.dll" "$OUT_DIR/styles/"
    echo "Copiado: styles/qwindowsvistastyle.dll"
fi

# MinGW runtime DLLs
for dll in libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll; do
    if [ -f "$DLL_DIR/$dll" ]; then
        cp "$DLL_DIR/$dll" "$OUT_DIR/"
        echo "Copiado: $dll"
    fi
done

# OpenCV e dependencias (recursivo)
echo "Resolvendo dependencias..."
declare -A COPIED

process_deps() {
    local BIN="$OUT_DIR/$1"
    [ ! -f "$BIN" ] && return
    local DEPS=$(objdump -p "$BIN" 2>/dev/null | grep "DLL Name" | awk '{print $3}')
    for dll in $DEPS; do
        case "$dll" in
            KERNEL32*|USER32*|GDI32*|ADVAPI32*|SHELL32*|OLE32*|OLEAUT32*|COMDLG32*|MSVCRT*|WS2_32*|SHLWAPI*|IMM32*|WINSPOOL*|RPCRT4*|WINMM*|COMCTL32*|api-ms-win*|ntdll*|secur32*|CRYPT32*|D3D11*|DXGI*|MFPlat*|MF*|UxTheme*|dwmapi*|VERSION*|opengl32*|WINHTTP*|bcrypt*|DNSAPI*|IPHLPAPI*|cfgmgr32*|propsys*|windowscodecs*)
                continue ;;
        esac
        [ -n "${COPIED[$dll]}" ] && continue
        COPIED[$dll]=1
        if [ -f "$DLL_DIR/$dll" ]; then
            cp "$DLL_DIR/$dll" "$OUT_DIR/"
            echo "Copiado: $dll"
            process_deps "$dll"
        fi
    done
}

process_deps "extrator-movimento.exe"

echo ""
echo "=== Concluido! ==="
echo "Executavel em: $(pwd)/$OUT_DIR/extrator-movimento.exe"
echo "Total de DLLs: $(ls -1 "$OUT_DIR"/*.dll 2>/dev/null | wc -l)"
