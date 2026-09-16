cmake_minimum_required(VERSION 3.15)
project(CombatAssist LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── libKCD1 ──────────────────────────────────────────────────────────────────
# Clone from https://github.com/JerryYOJ/libKCD1 and pass -DRE_ROOT=/path/to/it

if(NOT DEFINED RE_ROOT)
    message(FATAL_ERROR
        "RE_ROOT must point to your libKCD1 checkout.\n"
        "  cmake ... -DRE_ROOT=/path/to/libKCD1")
endif()

if(NOT MSVC)
    # ── MinGW: compile libKCD1 sources that are normally linked from the game ──
    # Under MSVC this is handled by .buildenv/CMakeLists.txt; here we need to
    # build the runtime-address-lookup helpers ourselves so the plugin can link.
    file(GLOB COMBATMODULE_SOURCES "${RE_ROOT}/src/combatmodule/*.cpp")

    add_library(kcd_re STATIC
        ${RE_ROOT}/src/Offsets/Offsets.cpp
        ${RE_ROOT}/src/crysystem/SSystemGlobalEnvironment.cpp
        ${RE_ROOT}/src/game/S_GameContext.cpp
        ${RE_ROOT}/src/entitymodule/C_Actor.cpp
        ${RE_ROOT}/.buildenv/CrySDKStubs.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/stubs/mingw_cryengine_stubs.cpp
        ${COMBATMODULE_SOURCES})

    # cmake/compat must come FIRST so its shims shadow SDK headers with the same
    # name (Windows.h, platform.h stub, IFlashUI.h stub, guimodule stubs, etc.)
    target_include_directories(kcd_re PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/compat
        ${RE_ROOT}/include
        ${RE_ROOT}/include/CryEngine/CryCommon)

    target_compile_definitions(kcd_re PRIVATE
        _ITERATOR_DEBUG_LEVEL=0 WIN32_LEAN_AND_MEAN NOMINMAX)
    target_compile_options(kcd_re PRIVATE
        -Wall -Wno-unused-parameter -fpermissive -Wno-deprecated-declarations)
    target_precompile_headers(kcd_re PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/PCH.h)
else()
    # MSVC: headers-only; the .buildenv/ build provides compiled objects.
    add_library(kcd_re INTERFACE)
    target_include_directories(kcd_re INTERFACE
        ${RE_ROOT}/include
        ${RE_ROOT}/include/CryEngine/CryCommon)
endif()

# ── CombatAssist plugin DLL ───────────────────────────────────────────────────

file(GLOB_RECURSE PLUGIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

if(NOT MSVC)
    # Provide pthread symbols statically so the final DLL doesn't import
    # libwinpthread-1.dll (which Proton/Wine doesn't supply).
    list(APPEND PLUGIN_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/stubs/winpthread_stub.cpp)
endif()

add_library(CombatAssist SHARED ${PLUGIN_SOURCES})
# KCSE loads "CombatAssist.dll" — drop the "lib" prefix CMake adds on MinGW.
set_target_properties(CombatAssist PROPERTIES PREFIX "")

target_precompile_headers(CombatAssist PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/PCH.h)

target_compile_definitions(CombatAssist PRIVATE
    _ITERATOR_DEBUG_LEVEL=0 WIN32_LEAN_AND_MEAN NOMINMAX)

if(MSVC)
    target_compile_options(CombatAssist PRIVATE /utf-8 /W4 /wd4819 /wd4100)
else()
    target_compile_options(CombatAssist PRIVATE
        -Wall -Wno-unused-parameter
        -fpermissive
        -Wno-deprecated-declarations)
    # -fno-threadsafe-statics removes the winpthread guard on static-local init;
    # the plugin loads on WHGame's main thread so the race can never happen.
    target_compile_options(CombatAssist PRIVATE -fno-threadsafe-statics)
    target_compile_options(kcd_re PRIVATE -fno-threadsafe-statics)
    target_link_options(CombatAssist PRIVATE
        -static-libgcc -static-libstdc++)
endif()

target_include_directories(CombatAssist PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(CombatAssist PRIVATE kcd_re)
