#include "DXShader.h"

#include <Lemon/Core.h>
#include <d3dcompiler.h>
#include <magic_enum/magic_enum.hpp>
#include <ranges>

namespace Lemon::DX
{
    const char* ShaderTypeToEntryPointName(const RHI::ShaderStage type)
    {
        switch (type)
        {
        case RHI::ShaderStage::Vertex:
            return "VSMain";
        case RHI::ShaderStage::Pixel:
            return "PSMain";
        default:
            LM_CORE_FATAL("Shader type unimplemented: {0}", magic_enum::enum_name(type));
            return "";
        }
    }
    const char* ShaderTypeToEntryTarget(const RHI::ShaderStage type)
    {
        switch (type)
        {
        case RHI::ShaderStage::Vertex:
            return "vs_5_1";
        case RHI::ShaderStage::Pixel:
            return "ps_5_1";
        default:
            LM_CORE_FATAL("Shader type unimplemented: {0}", magic_enum::enum_name(type));
            return "";
        }
    }

    DXShader::DXShader(const std::wstring& filePath, const RHI::ShaderStage type) : IShader()
    {
        DXShader::Compile(filePath, type);
    }

    void DXShader::Compile(const std::wstring& filePath, const RHI::ShaderStage type)
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

    const void* DXShader::GetBytecode()
    {
        return m_Handle->GetBufferPointer();
    }

    size_t DXShader::GetLength()
    {
        return m_Handle->GetBufferSize();
    }
}
