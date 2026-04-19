#include <Lemon.h>

class ExampleLayer : public Lemon::Layer {
public:
	ExampleLayer() : Layer("Example Layer") {}

	void OnUpdate() override {
	}

	void OnEvent(Lemon::Event &event) override {
	}
};

class Sandbox : public Lemon::Application {

public:

	Sandbox() {
		PushLayer(new ExampleLayer());
	}
	~Sandbox() {}
};


Lemon::Application* Lemon::CreateApplication() {
	return new Sandbox();

}