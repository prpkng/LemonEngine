#include "RHITypes.h"
#include "Lemon/Renderer/RHI/Helpers/RHIHelpers.h"



void Lemon::RHI::VertexAttribute::BuildSemanticName() {
    _semanticName_ = GetSemanticName(semantic);
}
