#pragma once

#ifdef LM_PLATFORM_WINDOWS 

extern Lemon::Application* Lemon::CreateApplication();

int main(int argc, char** argv) {
	auto application = Lemon::CreateApplication();
	application->Run();
	delete application;
}

#endif

