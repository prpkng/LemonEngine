#include <Lemon.h>

class Sandbox : public Lemon::Application {

public:

	Sandbox() = default;
	~Sandbox() {}
};


Lemon::Application* Lemon::CreateApplication() {

	LM_INFO("Creating Sandbox Application!");

	LM_FATAL("Failed to create Sandbox Application!");

	return new Sandbox();
}