#pragma once

#include "Core.h"

namespace Lemon {

	class LEMON_API Application
	{
	public:
		Application();
		~Application();
		
		void Run();

	};

	//To be defined in client
	Application* CreateApplication();

}

