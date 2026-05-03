#include "RHIHelpers.h"
#include "magic_enum/magic_enum.hpp"

namespace Lemon::RHI
{
[[nodiscard]] std::string GetSemanticName(Lemon::RHI::Semantic semantic) noexcept
{
    switch (semantic) {

    case Semantic::Position:
        return "POSITION";
    case Semantic::Normal:
        return "NORMAL";
    case Semantic::TexCoord0:
    case Semantic::TexCoord1:
    case Semantic::TexCoord2:
    case Semantic::TexCoord3:
    case Semantic::TexCoord4:
    case Semantic::TexCoord5:
    case Semantic::TexCoord6:
    case Semantic::TexCoord7:
        return "TEXCOORD";
    case Semantic::Color:
        return "COLOR";
    case Semantic::Tangent:
        return "TANGENT";
    // case Semantic::BoneIndices:
    //     return "BONEINDICES";
    // case Semantic::BoneWeights:
    //     return "BONEWEIGHTS";
    default:
        LM_CORE_FATAL("Semantic unimplemented: {0}", magic_enum::enum_name(semantic));
    }
    return "";
}

[[nodiscard]] u32 GetSemanticIndex(Lemon::RHI::Semantic semantic) noexcept
{
    switch (semantic) {

    case Semantic::TexCoord0:
        return 0;
    case Semantic::TexCoord1:
        return 1;
    case Semantic::TexCoord2:
        return 2;
    case Semantic::TexCoord3:
        return 3;
    case Semantic::TexCoord4:
        return 4;
    case Semantic::TexCoord5:
        return 5;
    case Semantic::TexCoord6:
        return 6;
    case Semantic::TexCoord7:
        return 7;
    case Semantic::Position:
    case Semantic::Normal:
    case Semantic::Color:
    case Semantic::Tangent:
        return 0;
    // case Semantic::BoneIndices:
    //     return "BONEINDICES";
    // case Semantic::BoneWeights:
    //     return "BONEWEIGHTS";
    default:
        LM_CORE_FATAL("Semantic unimplemented: {0}", magic_enum::enum_name(semantic));
    }
    return 0;
}
} // namespace Lemon::RHI