#pragma once

#include "lmpch.h"
#include "Lemon/Window.h"

class SDL_Window;

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

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
		
		SDL_Window* m_Handle;
		WindowData m_Data;
	};
}
