#pragma once
#include "../../Buffers/VertexBuffer.h"
#include "DXBuffer.h"

namespace Lemon::DX
{
    class DXVertexBuffer : public RHI::VertexBuffer
    {
    public:
        DXVertexBuffer(DXDevice* device, const Desc& desc);

        const std::shared_ptr<RHI::Buffer> GetBuffer() const override { return m_Buffer; }
        const D3D12_VERTEX_BUFFER_VIEW* GetBufferView() const { return &m_VertexBufferView; }
    private:
        std::shared_ptr<RHI::Buffer> m_Buffer;
        D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    };
} // Lemon::DX


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