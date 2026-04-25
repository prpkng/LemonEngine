#pragma once

#include <lmpch.h>

#include "IDevice.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Lemon::RHI
{

std::shared_ptr<ITexture> IDevice::LoadTexture(std::string_view path, Format format , u32 mipLevels)
{
    int width, height, channels;
    std::string pathStr(path);
    const u8* data = stbi_load(pathStr.c_str(), &width, &height, &channels, 4);

    
    ITexture::Desc texDesc(width, height, format, mipLevels);
    auto texture = CreateTexture(texDesc);

    auto uploadContext = CreateUploadContext();

    size_t size = width * height * channels;
    uploadContext->UploadTexture(*texture, {reinterpret_cast<const std::byte*>(data), size}, width * channels);

    uploadContext->Flush();

    return texture;
}

} // namespace Lemon::RHI