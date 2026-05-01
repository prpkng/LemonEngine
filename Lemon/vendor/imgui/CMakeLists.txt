cmake_minimum_required(VERSION 3.12)

project(imgui)

set(SOURCES
    imgui/imconfig.h
    imgui/imgui.h
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    
    imgui/backends/imgui_impl_dx12.h
    imgui/backends/imgui_impl_dx12.cpp
    imgui/backends/imgui_impl_sdl3.h
    imgui/backends/imgui_impl_sdl3.cpp
    )

add_library(imgui ${SOURCES})
target_link_libraries(imgui PRIVATE 
        SDL3::SDL3 
        d3d12          # DirectX 12
        dxgi           # DirectX Graphics Infrastructure
        dxguid         # DirectX Graphics Infrastructure
        d3dcompiler    # DirectX shader compilation 
        )

target_include_directories(imgui PUBLIC imgui/)

# Use generator expressions to force Release-like flags regardless of the global config
target_compile_options(imgui PRIVATE 
    $<$<CONFIG:Debug>:-O3> # Add optimization even in Debug
    $<$<CONFIG:Debug>:-DNDEBUG> # Disable assertions
)

# # If using MSVC, you may need to specifically disable debug info generation for this target
if(MSVC)
    set_target_properties(imgui PROPERTIES COMPILE_FLAGS "/O2")
endif()



