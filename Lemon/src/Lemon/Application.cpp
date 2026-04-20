#include <lmpch.h>

#include "Application.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"
#include "Renderer/TestDXLayer.h"

namespace Lemon {
	
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application::Application()
	{
		LM_CORE_INFO("Starting application...");
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

		PushLayer(new TestDXLayer(m_Window));
	}
	
	Application::~Application()
	{

	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		// Iterate backwards through layer stack, so the foremost layer gets the input events first then the latter
		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin(); ) {
			(*--it)->OnEvent(e);
			if (e.IsHandled()) break;
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	void Application::PushLayer(Layer *layer) {
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer *overlay) {
		m_LayerStack.PushOverlay(overlay);
	}

	void Application::Run()
	{
		while (m_Running) {
			for (auto layer : m_LayerStack)
				layer->OnUpdate();

			m_Window->OnUpdate();
		}
	}

}


