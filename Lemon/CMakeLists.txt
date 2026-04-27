cmake_minimum_required(VERSION 3.10)

project(Lemon)

SET (CMAKE_COLOR_DIAGNOSTICS ON)
set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/SDL)
add_subdirectory(vendor/magic_enum)

add_subdirectory(vendor/assimp)

# add_subdirectory(vendor/DirectXTK12)

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
 "src/Lemon/LayerStack.cpp"
 "src/Lemon/LayerStack.h"
 "src/Lemon/Renderer/TestDXLayer.cpp"
 "src/Lemon/Renderer/TestDXLayer.h"
 "src/Lemon/Renderer/RHI/Helpers/Builders.h"
 "src/Lemon/Renderer/RHI/Helpers/Builders.cpp"
 "src/Lemon/Renderer/RHI/Types/RHICommandTypes.h"
 "src/Lemon/Renderer/RHI/Types/RHIEnums.h"
 "src/Lemon/Renderer/RHI/Types/RHITypes.h"
 "src/Lemon/Renderer/RHI/Interfaces/IFence.h"
 "src/Lemon/Renderer/RHI/Interfaces/ICommandList.h"
 "src/Lemon/Renderer/RHI/Interfaces/ICommandQueue.h"
 "src/Lemon/Renderer/RHI/Interfaces/IUploadContext.h"
 "src/Lemon/Renderer/RHI/Interfaces/IDevice.h"
 "src/Lemon/Renderer/RHI/Interfaces/IPipeline.h"
 "src/Lemon/Renderer/RHI/Interfaces/IShader.h"
 "src/Lemon/Renderer/RHI/Interfaces/IBuffer.h"
 "src/Lemon/Renderer/RHI/Interfaces/ITexture.h"
 

 "src/Lemon/Renderer/Backends/DX/API/Helpers.cpp"
 "src/Lemon/Renderer/Backends/DX/API/Helpers.h"
 "src/Lemon/Renderer/Backends/DX/API/DXRootSignatureDesc.h"
 "src/Lemon/Renderer/Backends/DX/API/DXRootSignatureDesc.cpp"
 "src/Lemon/Renderer/Backends/DX/API/DXGraphicsPSODesc.cpp"
 "src/Lemon/Renderer/Backends/DX/API/DXGraphicsPSODesc.h"
 "src/Lemon/Renderer/Backends/DX/API/DXPSO.cpp"
 "src/Lemon/Renderer/Backends/DX/API/DXPSO.h"

 "src/Lemon/Renderer/Backends/DX/DXDevice.cpp"
 "src/Lemon/Renderer/Backends/DX/DXDevice.h"
 "src/Lemon/Renderer/Backends/DX/Resources/DXBuffer.cpp"
 "src/Lemon/Renderer/Backends/DX/Resources/DXBuffer.h"
 "src/Lemon/Renderer/Backends/DX/Resources/DXSwapchain.h"
 "src/Lemon/Renderer/Backends/DX/Resources/DXSwapchain.cpp"
 "src/Lemon/Renderer/Backends/DX/Resources/DXTexture.cpp"
 "src/Lemon/Renderer/Backends/DX/Resources/DXTexture.h"
 "src/Lemon/Renderer/Backends/DX/Resources/DXUploadContext.h"
 "src/Lemon/Renderer/Backends/DX/Resources/DXUploadContext.cpp"
 "src/Lemon/Renderer/Backends/DX/Pipelines/DXShader.cpp"
 "src/Lemon/Renderer/Backends/DX/Pipelines/DXShader.h"
 "src/Lemon/Renderer/Backends/DX/Pipelines/DXPipeline.h"
 "src/Lemon/Renderer/Backends/DX/Commands/DXCommandQueue.h"
 "src/Lemon/Renderer/Backends/DX/Commands/DXCommandQueue.cpp"
 "src/Lemon/Renderer/Backends/DX/Commands/DXCommandList.cpp"
 "src/Lemon/Renderer/Backends/DX/Commands/DXCommandList.h"
 src/Lemon/Renderer/RHI/Interfaces/IDevice.cpp
 )

target_compile_features(Lemon PUBLIC cxx_std_23)
target_include_directories(Lemon PUBLIC src/ vendor/dx-headers vendor/include)

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
        assimp
        d3d12          # DirectX 12
        dxgi           # DirectX Graphics Infrastructure
        dxguid         # DirectX Graphics Infrastructure
        d3dcompiler    # DirectX shader compilation
        # DirectXTK12     # DirectX Tool Kit for DirectX 12
)


if(${WIN32})
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}> $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
  COMMAND_EXPAND_LISTS
)
endif()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t
        ${CMAKE_BINARY_DIR}/Sandbox/
        $<TARGET_FILE:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
)