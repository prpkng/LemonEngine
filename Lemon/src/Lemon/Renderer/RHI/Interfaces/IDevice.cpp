#pragma once

#include <lmpch.h>

#include "IDevice.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Lemon::RHI
{

ITexture* IDevice::LoadTexture(std::string_view path, Format format , u32 mipLevels)
{
    int width, height, channels;
    std::string pathStr(path);
    const u8* data = stbi_load(pathStr.c_str(), &width, &height, &channels, 4);

    
    ITexture::Desc texDesc(width, height, format, mipLevels);

    size_t lastSep = path.find_last_of('/');
    texDesc.debugName = path.substr(lastSep + 1);

    auto texture = CreateTexture(texDesc);

    auto uploadContext = CreateUploadContext();

    size_t size = width * height * channels;
    uploadContext->UploadTexture(*texture, {reinterpret_cast<const std::byte*>(data), size}, width * channels);

    uploadContext->Flush();

    return texture;
}

} // namespace Lemon::RHI