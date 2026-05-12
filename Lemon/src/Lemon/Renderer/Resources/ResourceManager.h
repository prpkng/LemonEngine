#pragma once

#include "Lemon/Renderer/Backends/DX/Resources/DXTexture.h"
#include "Lemon/Renderer/RHI/Interfaces/ITexture.h"
#include "ResourcePool.h"
#include "ResourceRegistry.h"
#include <string>
namespace Lemon {

using AssetID = std::string;

template <typename Handle>
struct AssetRecord {
    AssetID asset;

    Handle handle;

    enum class State {
        Unloaded,
        Loading,
        Loaded
    };

    State state = State::Unloaded;

    u64 version = 0;

    // std::string name;
};

class ResourceManager {
public:
    TextureHandle CreateTexture(const RHI::ITexture::Desc& desc);
    TextureHandle LoadTexture(std::string_view path);
    
    RHI::ITexture* GetTexture(TextureHandle handle);

private:

    ResourceRegistry<DX::DXTexture, TextureTag, 1024, AssetID> m_TextureRegistry;


    
};

}