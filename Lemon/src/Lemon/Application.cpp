#include <lmpch.h>

#include "Application.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"

namespace Lemon {
	
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application::Application()
	{
		LM_CORE_INFO("Starting application...");
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
	}
	
	Application::~Application()
	{

	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		LM_CORE_TRACE("{0}", e.ToString());
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	void Application::Run()
	{
		while (m_Running) {
			m_Window->OnUpdate();
		}
	}

}


