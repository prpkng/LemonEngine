#include <Lemon.h>

class Sandbox : public Lemon::Application {

public:

	Sandbox() = default;
	~Sandbox() {}
};


Lemon::Application* Lemon::CreateApplication() {
	return new Sandbox();

}