#pragma once
#include <Lemon/Renderer/Pipelines/ShaderPiece.h>

#include "DXUtils.h"
#include "d3dcommon.h"

namespace Lemon::DX
{
    class DXShaderPiece : public RHI::ShaderPiece
    {
    public:
        DXShaderPiece(const std::wstring& filePath, RHI::ShaderPieceType type);

        void Compile(const std::wstring& filePath, RHI::ShaderPieceType type) override;

        const void* GetBytecode() override;
        size_t GetLength() override;
    private:
        ComPtr<ID3DBlob> m_Handle;
    };
}
