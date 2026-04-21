#pragma once

#include <lmpch.h>
#include <Lemon/Renderer/RHI/Types/RHITypes.h>

namespace Lemon::RHI
{


struct IShader
{
    virtual ~IShader() = default;

    virtual void Compile(const std::wstring& filePath, ShaderStage type) = 0;
    virtual size_t GetLength() = 0;
    virtual const void* GetBytecode() = 0;
};
}