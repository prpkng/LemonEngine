#pragma once

#include "Lemon/Window.h"
#include "SDL3/SDL_events.h"

struct SDL_Window;

namespace Lemon {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		// Window attributes
		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			bool VSync;
			EventCallbackFn EventCallback;
		};

		SDL_Window* m_Handle;
	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

		WindowData m_Data;
	};
}
