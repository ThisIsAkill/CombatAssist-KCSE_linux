# MinGW-w64 cross-compilation toolchain for building Windows DLLs on Linux.
# Produces a CombatAssist.dll that runs under Wine/Proton on Linux.
#
# Prerequisites:
#   sudo pacman -S mingw-w64-gcc   (Arch/CachyOS)
#   sudo apt install mingw-w64     (Debian/Ubuntu)
#
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw64.cmake -DRE_ROOT=/path/to/libKCD1 .

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER  x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
