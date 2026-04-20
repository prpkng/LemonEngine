#include <lmpch.h>
#include "WindowsWindow.h"

#include <SDL3/SDL.h>
#include <Lemon/Events/Event.h>
#include <Lemon/Events/ApplicationEvent.h>
#include <Lemon/Events/MouseEvent.h>
#include <Lemon/Events/KeyEvent.h>

namespace Lemon {
	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}
	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		WindowsWindow::Init(props);
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		if (not SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
			LM_CORE_FATAL("Failed to initialize SDL: {0}", SDL_GetError());
			return;
		}

		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.Title = props.Title;

		SDL_Window* window = SDL_CreateWindow(
			m_Data.Title.c_str(), m_Data.Width, m_Data.Height, SDL_WINDOW_RESIZABLE);

		LM_CORE_ASSERT(window != nullptr, "Failed to create window: {0}", SDL_GetError());

		m_Handle = window;

		LM_CORE_INFO("Created window {0} ({1}, {2})", m_Data.Title, m_Data.Width, m_Data.Height);

		SDL_ShowWindow(window);


	}

	WindowsWindow::~WindowsWindow()
	{
		WindowsWindow::Shutdown();
	}

	void WindowsWindow::OnUpdate()
	{
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type) {
			case SDL_EVENT_QUIT:
			{
				auto ev = Lemon::WindowCloseEvent();
				m_Data.EventCallback(ev);
				break;
			}
			case SDL_EVENT_WINDOW_RESIZED:
			{
				m_Data.Width = e.window.data1;
				m_Data.Height = e.window.data2;
				auto ev = Lemon::WindowResizeEvent(m_Data.Width, m_Data.Height);
				m_Data.EventCallback(ev);
				break;
			}
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				auto ev = Lemon::MouseButtonPressedEvent((int)e.button.button);
				m_Data.EventCallback(ev);
				break;
			}
			case SDL_EVENT_MOUSE_BUTTON_UP:
			{
				auto ev = Lemon::MouseButtonReleasedEvent((int)e.button.button);
				m_Data.EventCallback(ev);
				break;
			}
			case SDL_EVENT_MOUSE_MOTION:
			{
				auto ev = Lemon::MouseMovedEvent((float)e.motion.x, (float)e.motion.y);
				m_Data.EventCallback(ev);
				break;
			}
			case SDL_EVENT_KEY_DOWN:
			{
				auto ev = Lemon::KeyPressedEvent((int)e.key.key, e.key.repeat);
				m_Data.EventCallback(ev);
				break;
			}
			case SDL_EVENT_KEY_UP:
			{
				auto ev = Lemon::KeyReleasedEvent((int)e.key.key, e.key.repeat);
				m_Data.EventCallback(ev);
				break;
			}
			default:
				break;
			}
		}
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
	}

	bool WindowsWindow::IsVSync() const
	{
		return false;
	}

	void WindowsWindow::Shutdown()
	{
		if (m_Handle) {
			SDL_DestroyWindow(static_cast<SDL_Window*>(m_Handle));
			m_Handle = nullptr;
		}
	}
}