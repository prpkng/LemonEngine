#include "DXShaderPiece.h"

#include <Lemon/Core.h>
#include <d3dcompiler.h>
#include <magic_enum/magic_enum.hpp>
#include <ranges>

namespace Lemon::DX
{
    const char* ShaderTypeToEntryPointName(const RHI::ShaderPieceType type)
    {
        switch (type)
        {
        case RHI::ShaderPieceType::Vertex:
            return "VSMain";
        case RHI::ShaderPieceType::Pixel:
            return "PSMain";
        default:
            LM_CORE_FATAL("Shader type unimplemented: {0}", magic_enum::enum_name(type));
            return "";
        }
    }
    const char* ShaderTypeToEntryTarget(const RHI::ShaderPieceType type)
    {
        switch (type)
        {
        case RHI::ShaderPieceType::Vertex:
            return "vs_5_0";
        case RHI::ShaderPieceType::Pixel:
            return "ps_5_0";
        default:
            LM_CORE_FATAL("Shader type unimplemented: {0}", magic_enum::enum_name(type));
            return "";
        }
    }

    DXShaderPiece::DXShaderPiece(const std::wstring& filePath, const RHI::ShaderPieceType type) : ShaderPiece()
    {
        DXShaderPiece::Compile(filePath, type);
    }

    void DXShaderPiece::Compile(const std::wstring& filePath, const RHI::ShaderPieceType type)
    {
        ComPtr<ID3DBlob> errorMsgBlob = nullptr;
        const HRESULT hr = D3DCompileFromFile(
            filePath.c_str(),
            nullptr,
            nullptr,
            ShaderTypeToEntryPointName(type),
            ShaderTypeToEntryTarget(type),
            NULL,
            NULL,
            &m_Handle,
            &errorMsgBlob
            );

        if (FAILED(hr))
        {
            auto typeName = std::string(magic_enum::enum_name(type));
            std::ranges::transform(typeName, typeName.begin(), [](unsigned char c) {
                return std::tolower(c);
            });

            std::string errorMessage(static_cast<const char*>(errorMsgBlob->GetBufferPointer()), errorMsgBlob->GetBufferSize());

            LM_CORE_ERROR("Failed to compile {0} shader!\n{1}", typeName, errorMessage);
        }
    }

    const void* DXShaderPiece::GetBytecode()
    {
        return m_Handle->GetBufferPointer();
    }

    size_t DXShaderPiece::GetLength()
    {
        return m_Handle->GetBufferSize();
    }
}
