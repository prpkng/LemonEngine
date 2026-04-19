#include <lmpch.h>

#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Lemon {

	Log::Log()
	{
#ifdef NDEBUG
		spdlog::set_pattern("%^[%T] [%l] %n: %v%$");
#else
		spdlog::set_pattern("%^[%T] [%l] - [%s:%# %!()] %n: %v%$");
#endif


		m_CoreLogger = spdlog::stdout_color_mt("LEMON");
		m_CoreLogger->set_level(spdlog::level::trace);

		m_ClientLogger = spdlog::stdout_color_mt("APP"); 
		m_ClientLogger->set_level(spdlog::level::trace);

		
	}

	Log::~Log()
	{
	}

	Log& Log::Instance()
	{
		static Log logger{};

		return logger;
	}
}