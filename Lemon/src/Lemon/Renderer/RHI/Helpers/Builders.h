#pragma once

#include "Lemon/Renderer/RHI/Types/RHITypes.h"
#include <Lemon/Core.h>
#include <Lemon/Renderer/RHI/Interfaces/IBuffer.h>
#include <vector>


namespace Lemon::RHI
{
    class VertexLayoutBuilder
    {
    public:
        explicit VertexLayoutBuilder();
        VertexLayoutBuilder& WithElement(const std::string& semanticName, ElementType type, bool addToAccumulation = true);
        [[nodiscard]] VertexLayout Build(InputRate inputRate = InputRate::PerVertex);

    private:
        u32 m_AccumulatedOffset;
        u32 m_TotalStride;
        std::vector<VertexElement> m_Elements{};
    };

    class InputLayoutBuilder
    {
    public:
        explicit InputLayoutBuilder();
        InputLayoutBuilder& WithElement(const std::string& semanticName, ElementType type, InputRate inputRate = InputRate::PerVertex, u32 semanticIndex = 0);
        [[nodiscard]] std::vector<VertexAttribute> Build();

    private:
        u32 m_ElementCount;
        u32 m_AccumulatedOffset;
        std::vector<VertexAttribute> m_Attributes{};
    };

    u32 GetVertexElementSize(const ElementType type);
}

