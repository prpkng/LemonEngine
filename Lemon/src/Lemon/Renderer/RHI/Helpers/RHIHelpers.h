#pragma once

#include "Lemon/Renderer/RHI/Types/RHITypes.h"

namespace Lemon::RHI
{

[[nodiscard]] std::string GetSemanticName(Lemon::RHI::Semantic semantic) noexcept;
[[nodiscard]] u32 GetSemanticIndex(Lemon::RHI::Semantic semantic) noexcept;

}