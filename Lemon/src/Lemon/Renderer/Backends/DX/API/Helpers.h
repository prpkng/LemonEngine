#pragma once

#include <dxgi.h>
#include <Lemon/Renderer/RHI/Types/RHITypes.h>
#include <Lemon/Core.h>

#include <wrl/client.h>

#include "Lemon/Renderer/RHI/Types/RHICommandTypes.h"

#define CHECK(x, msg) { HRESULT __hr__ = x; if (FAILED(__hr__)) { LM_CORE_FATAL("{0}: {1}", msg, HrToString(__hr__)); abort(); } }

using Microsoft::WRL::ComPtr;

std::string HrToString(HRESULT hr);

DXGI_FORMAT TranslateElementTypeToFormat(Lemon::RHI::ElementType type);

// ===============================================
// Translation helpers - RHI -> DirectX 12
// ===============================================

namespace Lemon::DX::Convert
{
    using namespace RHI;

    [[nodiscard]] constexpr D3D12_SHADER_VISIBILITY ToVisibility(const ShaderStage s) noexcept {
        switch (s) {
        case ShaderStage::Vertex: return D3D12_SHADER_VISIBILITY_VERTEX;
        case ShaderStage::Pixel:  return D3D12_SHADER_VISIBILITY_PIXEL;
        default:                  return D3D12_SHADER_VISIBILITY_ALL;
        }
    }

    [[nodiscard]] constexpr D3D12_PRIMITIVE_TOPOLOGY_TYPE ToTopologyType(const PrimitiveTopology t) noexcept {
        switch (t) {
        case PrimitiveTopology::TriangleList:
        case PrimitiveTopology::TriangleFan:
        case PrimitiveTopology::TriangleStrip: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        case PrimitiveTopology::LineStrip:
        case PrimitiveTopology::LineList:      return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case PrimitiveTopology::PointList:     return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        default:                               return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
        }
    }
    [[nodiscard]] constexpr D3D_PRIMITIVE_TOPOLOGY ToTopology(const PrimitiveTopology t) noexcept {
        switch (t) {
        case PrimitiveTopology::TriangleList:  return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case PrimitiveTopology::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case PrimitiveTopology::TriangleFan: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLEFAN;
        case PrimitiveTopology::LineList: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case PrimitiveTopology::LineStrip: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case PrimitiveTopology::PointList: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        default: return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
        }
    }

    [[nodiscard]] constexpr D3D12_CULL_MODE ToCullMode(const CullMode m) noexcept {
        switch (m) {
        case CullMode::None:  return D3D12_CULL_MODE_NONE;
        case CullMode::Front: return D3D12_CULL_MODE_FRONT;
        case CullMode::Back:  return D3D12_CULL_MODE_BACK;
        default:              return D3D12_CULL_MODE_BACK;
        }
    }



    [[nodiscard]] constexpr D3D12_HEAP_TYPE ToHeapType(MemoryUsage usage)
    {
        switch (usage)
        {
        case MemoryUsage::GPU_ONLY:   return D3D12_HEAP_TYPE_DEFAULT;
        case MemoryUsage::CPU_TO_GPU: return D3D12_HEAP_TYPE_UPLOAD;
        case MemoryUsage::GPU_TO_CPU: return D3D12_HEAP_TYPE_READBACK;
        }
        return D3D12_HEAP_TYPE_DEFAULT;
    }

    [[nodiscard]] constexpr D3D12_RESOURCE_STATES ToResourceState(MemoryUsage usage)
    {
        switch (usage)
        {
        case MemoryUsage::GPU_ONLY:   return D3D12_RESOURCE_STATE_COMMON;
        case MemoryUsage::CPU_TO_GPU: return D3D12_RESOURCE_STATE_GENERIC_READ;
        case MemoryUsage::GPU_TO_CPU: return D3D12_RESOURCE_STATE_COPY_DEST;
        }
        return D3D12_RESOURCE_STATE_COMMON;
    }


    [[nodiscard]] constexpr DXGI_FORMAT ToFormat(const Format f) noexcept {
        switch (f) {
        case Format::RGBA8_UNORM:       return DXGI_FORMAT_R8G8B8A8_UNORM;
        case Format::RGBA16_FLOAT:      return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case Format::RGBA32_FLOAT:      return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case Format::D32_FLOAT:         return DXGI_FORMAT_D32_FLOAT;
        case Format::D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
        default:                        return DXGI_FORMAT_UNKNOWN;
        }
    }

    [[nodiscard]] constexpr DXGI_FORMAT ToFormat(const ElementType type) noexcept
    {
        switch (type)
        {
        case ElementType::Float:  return DXGI_FORMAT_R32_FLOAT;
        case ElementType::Float2: return DXGI_FORMAT_R32G32_FLOAT;
        case ElementType::Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
        case ElementType::Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;

        case ElementType::Int:  return DXGI_FORMAT_R32_SINT;
        case ElementType::Int2: return DXGI_FORMAT_R32G32_SINT;
        case ElementType::Int3: return DXGI_FORMAT_R32G32B32_SINT;
        case ElementType::Int4: return DXGI_FORMAT_R32G32B32A32_SINT;

        case ElementType::Uint:  return DXGI_FORMAT_R32_UINT;
        case ElementType::Uint2: return DXGI_FORMAT_R32G32_UINT;
        case ElementType::Uint3: return DXGI_FORMAT_R32G32B32_UINT;
        case ElementType::Uint4: return DXGI_FORMAT_R32G32B32A32_UINT;

        case ElementType::Ushort:  return DXGI_FORMAT_R16_UINT;
        case ElementType::Ushort2: return DXGI_FORMAT_R16G16_UINT;
        case ElementType::Ushort4: return DXGI_FORMAT_R16G16B16A16_UINT;

        case ElementType::Ubyte4:  return DXGI_FORMAT_R8G8B8A8_UINT;
        case ElementType::Ubyte4N:  return DXGI_FORMAT_R8G8B8A8_UNORM;
        case ElementType::Byte4:  return DXGI_FORMAT_R8G8B8A8_SINT;
        case ElementType::Byte4N:  return DXGI_FORMAT_R8G8B8A8_SNORM;


        default:
            LM_CORE_ERROR("Unknown ElementType");
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    [[nodiscard]] constexpr DXGI_FORMAT ToVertexFormat(const Format f) noexcept {
        return ToFormat(f); // reuse — same enum, different semantic use
    }

    [[nodiscard]] constexpr D3D12_COMMAND_LIST_TYPE ToCommandListType(const QueueType t) noexcept {
        switch (t) {
        case QueueType::Graphics: return D3D12_COMMAND_LIST_TYPE_DIRECT;
        case QueueType::Compute:  return D3D12_COMMAND_LIST_TYPE_COMPUTE;
        case QueueType::Copy:     return D3D12_COMMAND_LIST_TYPE_COPY;
        default:                  return D3D12_COMMAND_LIST_TYPE_DIRECT;
        }
    }

    [[nodiscard]] constexpr D3D12_RESOURCE_STATES ToResourceState(const ResourceState state) noexcept
    {
        switch (state)
        {
        case ResourceState::Present:
            return D3D12_RESOURCE_STATE_PRESENT;
        case ResourceState::RenderTarget:
            return D3D12_RESOURCE_STATE_RENDER_TARGET;
        case ResourceState::DepthWrite:
            return D3D12_RESOURCE_STATE_DEPTH_WRITE;
        case ResourceState::DepthRead:
            return D3D12_RESOURCE_STATE_DEPTH_READ;
        case ResourceState::ShaderResource:
            return D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE;
        case ResourceState::CopySource:
            return D3D12_RESOURCE_STATE_COPY_SOURCE;
        case ResourceState::CopyDest:
            return D3D12_RESOURCE_STATE_COPY_DEST;
        default:
            return D3D12_RESOURCE_STATE_PRESENT;
        }
    }



} // namespace RHI::DX::Convert