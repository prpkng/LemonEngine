cmake_minimum_required(VERSION 3.10)

set (CMAKE_CXX_STANDARD 23)

add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/SDL)
add_subdirectory(vendor/magic_enum)

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
        src/Lemon/Renderer/Backends/DX/API/DXUtils.h
        src/Lemon/Renderer/Backends/DX/DXBuffer.cpp
        src/Lemon/Renderer/Backends/DX/DXBuffer.h
        src/Lemon/Renderer/RHI/Interfaces/IBuffer.h
        src/Lemon/Renderer/Backends/DX/DXVertexBuffer.cpp
        src/Lemon/Renderer/Backends/DX/DXVertexBuffer.h
        src/Lemon/Renderer/RHI/Helpers/VertexLayoutBuilder.h
        src/Lemon/Renderer/RHI/Helpers/VertexLayoutBuilder.cpp
        src/Lemon/Renderer/Backends/DX/DXIndexBuffer.cpp
        src/Lemon/Renderer/Backends/DX/DXIndexBuffer.h
        src/Lemon/Renderer/RHI/Interfaces/IShader.h
        src/Lemon/Renderer/Backends/DX/DXShader.cpp
        src/Lemon/Renderer/Backends/DX/DXShader.h
        src/Lemon/Renderer/Backends/DX/API/DXUtils.cpp
        src/Lemon/Renderer/RHI/Types/RHITypes.h
        src/Lemon/Renderer/RHI/Interfaces/IPipeline.h
        src/Lemon/Renderer/Backends/DX/DXPipeline.h
        src/Lemon/Renderer/Backends/DX/API/TranslationHelpers.h
        src/Lemon/Renderer/Backends/DX/API/DXRootSignatureDesc.h
        src/Lemon/Renderer/Backends/DX/API/DXRootSignatureDesc.cpp
        src/Lemon/Renderer/Backends/DX/API/DXGraphicsPSODesc.cpp
        src/Lemon/Renderer/Backends/DX/API/DXGraphicsPSODesc.h
        src/Lemon/Renderer/Backends/DX/API/DXPSO.cpp
        src/Lemon/Renderer/Backends/DX/API/DXPSO.h
)

target_include_directories(Lemon PUBLIC src/)
target_include_directories(Lemon PUBLIC vendor/dx-headers)

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
        magic_enum::magic_enum
        d3d12          # DirectX 12
        dxgi           # DirectX Graphics Infrastructure
        d3dcompiler    # DirectX shader compilation
)


add_custom_command(TARGET Lemon POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/Lemon.dll ${CMAKE_BINARY_DIR}/Sandbox
        COMMAND_EXPAND_LISTS
)