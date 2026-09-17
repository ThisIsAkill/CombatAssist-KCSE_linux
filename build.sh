#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build-mingw"
TOOLCHAIN="${SCRIPT_DIR}/cmake/toolchain-mingw64.cmake"
RE_ROOT="${SCRIPT_DIR}/extern/libKCD1"

# Ensure submodule is populated
if [ ! -f "${RE_ROOT}/include/KCSE/KCSEAPI.h" ]; then
    echo "Initializing submodules..."
    git -C "${SCRIPT_DIR}" submodule update --init
fi

# libKCD1's RE'd struct layouts assume MSVC's std::map (16 bytes); libstdc++
# (GCC/MinGW) uses 48. Without this patch, several combatmodule structs fail
# their size static_asserts when cross-compiled.
#
# MSVC doesn't vtable-order same-name virtual overloads by declaration order
# the way Itanium/GCC does; IMessagingInterface::RegisterListener's two
# overloads land in the opposite vtable slots in every existing prebuilt
# (MSVC-built) plugin. Reordering the declarations to match MSVC's actual
# layout keeps a MinGW-built CombatAssist.dll binary-compatible with the
# loader (which carries the same fix).
for PATCH in \
    "${SCRIPT_DIR}/patches/libKCD1-mingw-struct-layout.patch" \
    "${SCRIPT_DIR}/patches/libKCD1-msvc-vtable-order.patch"
do
    if ! git -C "${RE_ROOT}" apply --reverse --check "${PATCH}" 2>/dev/null; then
        echo "Patching libKCD1: $(basename "${PATCH}")..."
        git -C "${RE_ROOT}" apply "${PATCH}"
    fi
done

# Configure (only if not already configured)
if [ ! -f "${BUILD_DIR}/build.ninja" ] && [ ! -f "${BUILD_DIR}/Makefile" ]; then
    echo "Configuring..."
    cmake -B "${BUILD_DIR}" \
        -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN}" \
        -DRE_ROOT="${RE_ROOT}"
fi

# Build
echo "Building..."
cmake --build "${BUILD_DIR}" --parallel

echo ""
echo "Output: ${BUILD_DIR}/CombatAssist.dll"
echo ""
echo "Deploy:  cp ${BUILD_DIR}/CombatAssist.dll ~/.steam/steam/steamapps/common/KingdomComeDeliverance/mods/CombatAssist/KCSE/Plugins/"
