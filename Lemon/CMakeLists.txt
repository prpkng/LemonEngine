cmake_minimum_required(VERSION 3.10)

set (CMAKE_CXX_STANDARD 23)

add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/SDL)

add_library(Lemon SHARED
 "src/Lemon.h"
 "src/Lemon/Application.h"
 "src/Lemon/Application.cpp"
 "src/Lemon/Core.h"
 "src/Lemon/EntryPoint.h"
 "src/Lemon/Log.h"
 "src/Lemon/Log.cpp"
 "src/Lemon/Window.h"
 "src/Lemon/Events/Event.h"
 "src/Lemon/Events/ApplicationEvent.h"
 "src/Lemon/Events/KeyEvent.h"
 "src/Lemon/Events/MouseEvent.h"
 "src/Platform/WindowsWindow.h"
 "src/Platform/WindowsWindow.cpp")

target_include_directories(Lemon PUBLIC src/)

target_precompile_headers(Lemon PRIVATE src/lmpch.h)

target_compile_definitions(Lemon 
    PRIVATE LM_BUILD_DLL
    PUBLIC $<$<PLATFORM_ID:Windows>:LM_PLATFORM_WINDOWS>
    PUBLIC $<$<CONFIG:Debug>:LM_DEBUG LM_ENABLE_ASSERTS SPDLOG_ACTIVE_LEVEL=TRACE>
    PUBLIC $<$<CONFIG:Release>:LM_RELEASE>
    PUBLIC $<$<CONFIG:DIST>:LM_DIST>
)

target_link_libraries(Lemon PUBLIC SDL3::SDL3 spdlog::spdlog)