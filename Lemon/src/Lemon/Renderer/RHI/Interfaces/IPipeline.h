#pragma once

#include <string>

#include "Lemon/Renderer/RHI/Types/RHITypes.h"

namespace Lemon::RHI
{

    struct IPipeline
    {
        struct Desc
        {
            // Shaders
            std::string vertexShaderPath;
            std::string pixelShaderPath;

            std::vector<VertexAttribute> inputLayout{};
            std::vector<RootParameter> rootParameters{};
            std::vector<StaticSamplerDesc> staticSamplers{};

            RasterizerState rasterizerState {};
            DepthStencilState depthStencilState {};
            BlendState blendState {};
            SampleState sampleState {};



            PrimitiveTopology topology = PrimitiveTopology::TriangleList;

            std::vector<Format> renderTargetFormats{};
            Format depthStencilFormat = Format::Unknown;
        };

        virtual ~IPipeline() = default;

    };
} // namespace Lemon::RHI
