#pragma once

#include "Lemon/Renderer/RHI/Interfaces/IBuffer.h"
#include "Lemon/Renderer/RHI/Types/RHITypes.h"
#include <Lemon/Core.h>


namespace Lemon {

struct Mesh {

    std::vector<std::shared_ptr<RHI::VertexBufferView>> vertexBuffers;

    std::shared_ptr<RHI::IndexBufferView> indexBuffer;

    RHI::VertexLayout vertexLayout;

};

}