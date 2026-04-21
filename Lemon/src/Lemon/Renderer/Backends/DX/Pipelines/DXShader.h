#pragma once
#include <Lemon/Renderer/RHI/Interfaces/IShader.h>

#include "../API/Helpers.h"
#include "d3dcommon.h"

namespace Lemon::DX
{
    class DXShader : public RHI::IShader
    {
    public:
        DXShader(const std::wstring& filePath, RHI::ShaderStage type);

        void Compile(const std::wstring& filePath, RHI::ShaderStage type) override;

        const void* GetBytecode() override;
        size_t GetLength() override;
    private:
        ComPtr<ID3DBlob> m_Handle;
    };
}
