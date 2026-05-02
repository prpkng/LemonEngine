#pragma once

#include <lmpch.h>
#include <Lemon/Core.h>
#include <Lemon/Events/Event.h>


namespace Lemon {
	class LEMON_API Layer
	{
	public:
		Layer(std::string name = "Layer") : m_DebugName(std::move(name)) {}
		virtual ~Layer() {}

		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void OnPreUpdate() {}
		virtual void OnUpdate() {}
		virtual void OnPostUpdate() {}

		virtual void OnPreRender() {}
		virtual void OnRender() {}
		virtual void OnPostRender() {}

		virtual void OnEvent(Event& event) {}
		

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
	};
}