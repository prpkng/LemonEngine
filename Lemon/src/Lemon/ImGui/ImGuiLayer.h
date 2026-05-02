#pragma once

#include "Lemon/Layer.h"
#include <Lemon/Core.h>

namespace Lemon
{

class ImGuiLayer : public Layer
{
public:
    ImGuiLayer();
    ~ImGuiLayer() override;

    void OnAttach() override;
    void OnDetach() override;

    void OnPreUpdate() override;
    void OnUpdate() override;
    void OnPostUpdate() override;

    void OnPreRender() override;
    void OnRender() override;
    void OnPostRender() override;

    void OnEvent(Event& e) override;

private:
    float m_Time = 0.0f;
};

} // namespace Lemon