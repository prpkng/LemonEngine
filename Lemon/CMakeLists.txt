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
 "src/Platform/WindowsWindow.cpp" 
 "src/Lemon/Layer.h" 
 "src/Lemon/Layer.cpp"
        src/Lemon/LayerStack.cpp
        src/Lemon/LayerStack.h
        src/Lemon/Renderer/TestDXLayer.cpp
        src/Lemon/Renderer/TestDXLayer.h
        src/Lemon/Renderer/Backends/DX/DXDevice.cpp
        src/Lemon/Renderer/Backends/DX/DXDevice.h
        src/Lemon/Renderer/Backends/DX/DXUtils.h
        src/Lemon/Renderer/Backends/DX/DXBuffer.cpp
        src/Lemon/Renderer/Backends/DX/DXBuffer.h
        src/Lemon/Renderer/Buffers/Buffer.h
        src/Lemon/Renderer/Buffers/VertexBuffer.h
        src/Lemon/Renderer/Backends/DX/DXVertexBuffer.cpp
        src/Lemon/Renderer/Backends/DX/DXVertexBuffer.h
        src/Lemon/Renderer/Helpers/VertexLayoutBuilder.h
        src/Lemon/Renderer/Helpers/VertexLayoutBuilder.cpp
        src/Lemon/Renderer/Buffers/IndexBuffer.h
        src/Lemon/Renderer/Backends/DX/DXIndexBuffer.cpp
        src/Lemon/Renderer/Backends/DX/DXIndexBuffer.h)

target_include_directories(Lemon PUBLIC src/)
target_include_directories(Lemon PRIVATE vendor/dx-headers)

target_precompile_headers(Lemon PRIVATE src/lmpch.h)

target_compile_definitions(Lemon 
    PRIVATE LM_BUILD_DLL
    PUBLIC $<$<PLATFORM_ID:Windows>:LM_PLATFORM_WINDOWS>
    PUBLIC $<$<CONFIG:Debug>:LM_DEBUG LM_ENABLE_ASSERTS SPDLOG_ACTIVE_LEVEL=TRACE>
    PUBLIC $<$<CONFIG:Release>:LM_RELEASE>
    PUBLIC $<$<CONFIG:DIST>:LM_DIST>
)
target_link_libraries(Lemon PUBLIC
        SDL3::SDL3     # For window handling
        spdlog::spdlog # For logging
        d3d12          # DirectX 12
        dxgi           # DirectX Graphics Infrastructure
        d3dcompiler    # DirectX shader compilation
)


add_custom_command(TARGET Lemon POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/Lemon.dll ${CMAKE_BINARY_DIR}/Sandbox
        COMMAND_EXPAND_LISTS
)