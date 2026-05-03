#pragma once

#include <Lemon/Core.h>
#include <Lemon/Renderer/RHI/Types/RHIEnums.h>
#include <Lemon/Renderer/RHI/Types/RHITypes.h>
#include <linalg.h>
#include <span>
#include <vector>
#include <unordered_map>

using namespace linalg::aliases;

namespace Lemon
{

struct AttributeStream {
    RHI::ElementType format;
    std::vector<u8>  data;

    AttributeStream() = default;
    AttributeStream(RHI::ElementType format) : format(format), data() {}
};

struct MeshAsset {
    std::unordered_map<RHI::Semantic, AttributeStream> attributes;
    std::vector<u32>                              indices;
};

// Remove this later
struct ImportVertex {
    float3 position;
    float3 normal;
    float2 uv;
};

} // namespace Lemon