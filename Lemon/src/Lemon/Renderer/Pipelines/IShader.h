#pragma once

#include <lmpch.h>
#include <Lemon/Renderer/RHITypes.h>

namespace Lemon::RHI
{

class IShader
{
public:
    virtual ~IShader() = default;

    virtual void Compile(const std::wstring& filePath, ShaderStage type) = 0;
    virtual size_t GetLength() = 0;
    virtual const void* GetBytecode() = 0;
};
}