//
// Created by guiej on 19/04/2026.
//

#include "LayerStack.h"

namespace Lemon {
    LayerStack::LayerStack() {
        m_LayerInsert = m_Layers.begin();
    }

    LayerStack::~LayerStack() {
        for (const auto* layer : m_Layers)
            delete layer;
    }

    void LayerStack::PushLayer(Layer *layer) {
        m_LayerInsert = m_Layers.insert(m_LayerInsert, layer);
        layer->OnAttach();
    }

    void LayerStack::PushOverlay(Layer *overlay) {
        m_Layers.emplace_back(overlay);
        overlay->OnAttach();
    }

    void LayerStack::PopLayer(Layer *layer) {
        if (const auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
            it != m_Layers.end()) {
            m_Layers.erase(it);
            --m_LayerInsert;
            layer->OnDetach();
        }

        LM_CORE_ERROR("Failed to detach layer: {0}", layer->GetName());
    }

    void LayerStack::PopOverlay(Layer *overlay) {
        if (const auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
            it != m_Layers.end()) {
            m_Layers.erase(it);
            overlay->OnDetach();
        }

        LM_CORE_ERROR("Failed to detach overlay layer: {0}", overlay->GetName());
    }
} // Lemon