#pragma once

#ifdef LM_PLATFORM_WINDOWS 

#include <SDL3/SDL_main.h>

extern Lemon::Application* Lemon::CreateApplication();

int main(int argc, char** argv) {
	auto application = Lemon::CreateApplication();
	application->Run();
	delete application;
	return 0;
}

#endif

