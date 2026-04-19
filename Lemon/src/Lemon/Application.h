#pragma once

#include "Core.h"
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
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;

	};

	//To be defined in client
	Application* CreateApplication();

}

