#pragma once

#include <lmpch.h>

namespace Lemon::RHI
{

enum class ShaderPieceType
{
    Vertex,
    Pixel,
    Geometry,
    Compute
};



class ShaderPiece
{
public:
    virtual ~ShaderPiece() = default;

    virtual void Compile(const std::wstring& filePath, RHI::ShaderPieceType type) = 0;
    virtual size_t GetLength() = 0;
    virtual const void* GetBytecode() = 0;
};
}