#pragma once

#include "Core.h"
#include "LayerStack.h"
#include "Window.h"

namespace Lemon {

	class LEMON_API Application
	{
	public:
		Application();
		~Application();
		
		void Run();
		
		void OnEvent(Event& e);

		bool OnWindowClose(class WindowCloseEvent& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;

	};

	//To be defined in client
	Application* CreateApplication();

}

