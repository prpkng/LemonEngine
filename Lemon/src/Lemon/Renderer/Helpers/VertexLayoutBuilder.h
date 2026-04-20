#pragma once
#include <lmpch.h>
#include <Lemon/Renderer/Buffers/VertexBuffer.h>


namespace Lemon::RHI
{
    class VertexLayoutBuilder
    {
    public:
        explicit VertexLayoutBuilder();
        VertexLayoutBuilder& WithElement(const std::string& name, VertexElementType type, bool addToAccumulation = true);
        VertexLayout Build(InputRate inputRate = InputRate::PerVertex);

    private:
        uint32_t m_AccumulatedOffset;
        uint32_t m_TotalStride;
        std::vector<VertexElement> m_Elements{};
    };
}
