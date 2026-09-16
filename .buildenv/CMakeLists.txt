project(CombatAssist LANGUAGES CXX)

file(GLOB_RECURSE PLUGIN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp")

add_library(CombatAssist SHARED ${PLUGIN_SOURCES})

target_precompile_headers(CombatAssist PRIVATE "${RE_BUILDENV}/PCH.h")

target_compile_definitions(CombatAssist PRIVATE
    _ITERATOR_DEBUG_LEVEL=0 WIN32_LEAN_AND_MEAN NOMINMAX)

if(MSVC)
    target_compile_options(CombatAssist PRIVATE /utf-8 /W4 /wd4819 /wd4100)
else()
    target_compile_options(CombatAssist PRIVATE
        -Wall -Wno-unused-parameter
        -finput-charset=utf-8 -fexec-charset=utf-8)
endif()

target_include_directories(CombatAssist PRIVATE
    ${RE_ROOT}/include ${RE_ROOT}/include/CryEngine/CryCommon ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(CombatAssist PRIVATE kcd_re spdlog::spdlog)

# set(PLUGIN_DEPLOY "C:/Program Files (x86)/Steam/steamapps/common/KingdomComeDeliverance/mods/Better_Kingdom/KCSE/Plugins")
# add_custom_command(TARGET CombatAssist POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:CombatAssist>" "${PLUGIN_DEPLOY}/"
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_PDB_FILE:CombatAssist>" "${PLUGIN_DEPLOY}/"
# )
