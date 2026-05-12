#include "ResourceManager.h"
#include "Lemon/Renderer/Renderer.h"


namespace Lemon {

TextureHandle ResourceManager::CreateTexture(const RHI::ITexture::Desc& desc) {
    DX::DXTexture texture = *reinterpret_cast<DX::DXTexture*>(Renderer::Instance().GetDevice()->CreateTexture(desc));
    return m_TextureRegistry.Insert(std::string(desc.debugName), std::move(texture)).value();
}

TextureHandle ResourceManager::LoadTexture(std::string_view path) {
    // auto handle = m_TextureDescPool.alloc(path, format, mipLevels);
    DX::DXTexture texture = *reinterpret_cast<DX::DXTexture*>(Renderer::Instance().GetDevice()->LoadTexture(path));
    return m_TextureRegistry.Insert(std::string(path), std::move(texture)).value();
}

RHI::ITexture* ResourceManager::GetTexture(TextureHandle handle) {
    return m_TextureRegistry.Get(handle);
}

}
