#pragma once

#include <lmpch.h>

#include "Lemon/Renderer/Buffer.h"
#include "Lemon/Renderer/VertexBuffer.h"


#define CHECK(x, msg) { HRESULT hr = x; if (FAILED(hr)) { LM_CORE_FATAL("{0}: {1}", msg, HrToString(hr)); abort(); } }

inline std::string HrToString(const HRESULT hr)
{
    switch (hr)
    {
        // Common success
        case S_OK: return "S_OK";

        // DXGI errors
        case DXGI_ERROR_DEVICE_REMOVED: return "DXGI_ERROR_DEVICE_REMOVED";
        case DXGI_ERROR_DEVICE_HUNG: return "DXGI_ERROR_DEVICE_HUNG";
        case DXGI_ERROR_DEVICE_RESET: return "DXGI_ERROR_DEVICE_RESET";
        case DXGI_ERROR_DRIVER_INTERNAL_ERROR: return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
        case DXGI_ERROR_INVALID_CALL: return "DXGI_ERROR_INVALID_CALL";
        case DXGI_ERROR_WAS_STILL_DRAWING: return "DXGI_ERROR_WAS_STILL_DRAWING";
        case DXGI_ERROR_FRAME_STATISTICS_DISJOINT: return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT";
        case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE";
        case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";
        case DXGI_ERROR_NOT_FOUND: return "DXGI_ERROR_NOT_FOUND";
        case DXGI_ERROR_MORE_DATA: return "DXGI_ERROR_MORE_DATA";
        case DXGI_ERROR_UNSUPPORTED: return "DXGI_ERROR_UNSUPPORTED";

        // D3D12 specific
        case D3D12_ERROR_ADAPTER_NOT_FOUND: return "D3D12_ERROR_ADAPTER_NOT_FOUND";
        case D3D12_ERROR_DRIVER_VERSION_MISMATCH: return "D3D12_ERROR_DRIVER_VERSION_MISMATCH";

        // Generic HRESULTs
        case E_FAIL: return "E_FAIL";
        case E_INVALIDARG: return "E_INVALIDARG";
        case E_OUTOFMEMORY: return "E_OUTOFMEMORY";
        case E_NOTIMPL: return "E_NOTIMPL";
        case E_POINTER: return "E_POINTER";

        default:
            break;
    }

    // Fallback: use Windows to get message
    char* msgBuf = nullptr;
    DWORD size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        hr,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&msgBuf,
        0,
        nullptr);

    std::ostringstream oss;

    if (size && msgBuf)
    {
        oss << msgBuf;
        LocalFree(msgBuf);
    }
    else
    {
        oss << "Unknown HRESULT";
    }

    // Append hex code (VERY useful)
    oss << " (0x" << std::hex << hr << ")";

    return oss.str();
}

inline D3D12_HEAP_TYPE TranslateHeapType(Lemon::RHI::MemoryUsage usage)
{
    switch (usage)
    {
    case Lemon::RHI::MemoryUsage::GPU_ONLY:   return D3D12_HEAP_TYPE_DEFAULT;
    case Lemon::RHI::MemoryUsage::CPU_To_GPU: return D3D12_HEAP_TYPE_UPLOAD;
    case Lemon::RHI::MemoryUsage::GPU_To_CPU: return D3D12_HEAP_TYPE_READBACK;
    }
    return D3D12_HEAP_TYPE_DEFAULT;
}

inline D3D12_RESOURCE_STATES TranslateInitialState(Lemon::RHI::MemoryUsage usage)
{
    switch (usage)
    {
    case Lemon::RHI::MemoryUsage::GPU_ONLY:   return D3D12_RESOURCE_STATE_COMMON;
    case Lemon::RHI::MemoryUsage::CPU_To_GPU: return D3D12_RESOURCE_STATE_GENERIC_READ;
    case Lemon::RHI::MemoryUsage::GPU_To_CPU: return D3D12_RESOURCE_STATE_COPY_DEST;
    }
    return D3D12_RESOURCE_STATE_COMMON;
}

inline DXGI_FORMAT TranslateElementType(Lemon::RHI::VertexElementType type)
{
    switch (type)
    {
    case Lemon::RHI::VertexElementType::Float:  return DXGI_FORMAT_R32_FLOAT;
    case Lemon::RHI::VertexElementType::Float2: return DXGI_FORMAT_R32G32_FLOAT;
    case Lemon::RHI::VertexElementType::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
    case Lemon::RHI::VertexElementType::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;

    case Lemon::RHI::VertexElementType::Int:  return DXGI_FORMAT_R32_SINT;
    case Lemon::RHI::VertexElementType::Int2: return DXGI_FORMAT_R32G32_SINT;
    case Lemon::RHI::VertexElementType::Int3: return DXGI_FORMAT_R32G32B32_SINT;
    case Lemon::RHI::VertexElementType::Int4: return DXGI_FORMAT_R32G32B32A32_SINT;

    case Lemon::RHI::VertexElementType::Uint:  return DXGI_FORMAT_R32_UINT;
    case Lemon::RHI::VertexElementType::Uint2: return DXGI_FORMAT_R32G32_UINT;
    case Lemon::RHI::VertexElementType::Uint3: return DXGI_FORMAT_R32G32B32_UINT;
    case Lemon::RHI::VertexElementType::Uint4: return DXGI_FORMAT_R32G32B32A32_UINT;

    case Lemon::RHI::VertexElementType::Ubyte4:  return DXGI_FORMAT_R8G8B8A8_UINT;
    case Lemon::RHI::VertexElementType::Ubyte4N:  return DXGI_FORMAT_R8G8B8A8_UNORM;
    case Lemon::RHI::VertexElementType::Byte4:  return DXGI_FORMAT_R8G8B8A8_SINT;
    case Lemon::RHI::VertexElementType::Byte4N:  return DXGI_FORMAT_R8G8B8A8_SNORM;


    default:
        LM_CORE_ERROR("Unknown VertexElementType");
        return DXGI_FORMAT_UNKNOWN;
    }
}