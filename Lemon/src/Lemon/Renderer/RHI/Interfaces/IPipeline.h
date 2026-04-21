#pragma once

#include <string>

#include "Lemon/Renderer/RHI/Types/RHITypes.h"

namespace Lemon::RHI
{


    // =================================================================================
    // Full PipelineDescriptor - 100% API-agnostic
    // =================================================================================
    struct IPipeline
    {
        struct Desc
        {
            // Shaders
            std::string vertexShaderPath;
            std::string pixelShaderPath;

            std::vector<VertexAttribute> inputLayout{};
            std::vector<RootParameter> rootParameters{};

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
