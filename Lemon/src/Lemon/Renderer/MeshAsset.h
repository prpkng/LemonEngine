#pragma once

#include <Lemon/Core.h>
#include <Lemon/Renderer/RHI/Types/RHIEnums.h>
#include <Lemon/Renderer/RHI/Types/RHITypes.h>
#include <linalg.h>
#include <span>
#include <unordered_map>
#include <vector>


using namespace linalg::aliases;

namespace Lemon
{

// struct AttributeStream {
//     RHI::ElementType format;
//     std::vector<u8>  data;

//     AttributeStream() = default;
//     AttributeStream(RHI::ElementType format) : format(format), data() {}
// };

struct MeshAsset {
    // std::unordered_map<RHI::Semantic, AttributeStream> attributes;
    std::vector<float3> positions;
    std::vector<float3> normals;
    std::vector<float2> uvs;
    std::vector<u32>    indices;
};

// Remove this later
struct ImportVertex {
    float3 position;
    float3 normal;
    float2 uv;
};

} // namespace Lemon