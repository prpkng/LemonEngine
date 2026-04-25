#pragma once

#include "Lemon/Renderer/RHI/Types/RHITypes.h"
#include <Lemon/Core.h>

namespace Lemon::RHI {

enum class TextureViewType : u32 {
    ShaderResource,  /// SRV - Used for sampling in shaders
    RenderTarget,    /// RTV - Render textures
    DepthStencil,    /// DSV - Depth / Stencil attachment
    UnorderedAccess, /// UAV - Compute read / write
};

enum class TextureAspect : u32 {
    Color   = BIT(0),
    Depth   = BIT(1),
    Stencil = BIT(2),
};

struct ITexture;

struct ITextureView {
    struct Desc {
        TextureViewType type       = TextureViewType::ShaderResource;
        TextureAspect   aspect     = TextureAspect::Color;
        Format          format     = Format::Unknown;
        u32             baseMip    = 0;
        u32             mipCount   = ~0u;
        u32             baseSlice  = 0;
        u32             sliceCount = 1;
    };

    virtual ~ITextureView() = default;

    [[nodiscard]] virtual TextureViewType        GetType() const = 0;
    [[nodiscard]] virtual const Desc& GetDesc() const = 0;

    [[nodiscard]] virtual const ITexture& GetTexture() const = 0;
};

struct ITexture {
    struct Desc {
        u32    width     = 1;
        u32    height    = 1;
        u32    mipLevels = 1;
        u32    arraySize = 1;
        Format format    = Format::RGBA8_UNORM;
        
        bool isRenderTarget       = false;
        bool isDepthStencil       = false;
        bool allowUnorderedAccess = false;
        std::string debugName;

        Desc() = default;
        Desc(u32 width, u32 height, Format format, u32 mipLevels = 1) : width(width), height(height), format(format), mipLevels(mipLevels) {}
    };

    virtual ~ITexture() = default;

    [[nodiscard]] virtual const Desc& GetDesc() const = 0;
    [[nodiscard]] virtual u32 GetWidth() const = 0;
    [[nodiscard]] virtual u32 GetHeight() const = 0;
    [[nodiscard]] virtual u32 GetMipLevels() const = 0;

    [[nodiscard]] virtual std::unique_ptr<ITextureView> CreateView(const ITextureView::Desc& desc) = 0;

    // ==== Convenience methods ===============

    /// Creates a Shader Resource View with default settings
    [[nodiscard]] std::unique_ptr<ITextureView> CreateSRV() {
        return CreateView({ .type = TextureViewType::ShaderResource });
    }

    /// Creates a Render Texture View with default settings and mip count
    /// @param mip Mip count
    [[nodiscard]] std::unique_ptr<ITextureView> CreateRTV(uint32_t mip = 0) {
        return CreateView({
            .type    = TextureViewType::RenderTarget,
            .baseMip = mip,
            .mipCount = 1,
        });
    }

    /// Creates a Depth / Stencil view with default settings
    [[nodiscard]] std::unique_ptr<ITextureView> CreateDSV() {
        return CreateView({
            .type   = TextureViewType::DepthStencil,
            .aspect = TextureAspect::Depth,
        });
    }
};

} // namespace Lemon::RHI