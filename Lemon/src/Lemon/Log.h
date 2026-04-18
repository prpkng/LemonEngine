#pragma once

#include "Core.h"
#include <spdlog/spdlog.h>
#include <memory>

namespace Lemon {

	class Log
	{
	public:
		LEMON_API Log();
		LEMON_API ~Log();

		LEMON_API inline std::shared_ptr<spdlog::logger>& GetCoreLogger() { return m_CoreLogger; }
		LEMON_API inline std::shared_ptr<spdlog::logger>& GetClientLogger() { return m_ClientLogger; }

		LEMON_API static Log& Instance();

	private:
		std::shared_ptr<spdlog::logger> m_CoreLogger;
		std::shared_ptr<spdlog::logger> m_ClientLogger;
	};

}

#ifdef SPDLOG_ACTIVE_LEVEL
#undef SPDLOG_ACTIVE_LEVEL
#endif

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
// Core log macros

#define LM_CORE_FATAL(...)     SPDLOG_LOGGER_CRITICAL(::Lemon::Log::Instance().GetCoreLogger(), __VA_ARGS__)
#define LM_CORE_ERROR(...)     SPDLOG_LOGGER_ERROR(::Lemon::Log::Instance().GetCoreLogger(), __VA_ARGS__)
#define LM_CORE_WARN(...)      SPDLOG_LOGGER_WARN(::Lemon::Log::Instance().GetCoreLogger(), __VA_ARGS__)
#define LM_CORE_INFO(...)      SPDLOG_LOGGER_INFO(::Lemon::Log::Instance().GetCoreLogger(), __VA_ARGS__)
#define LM_CORE_TRACE(...)     SPDLOG_LOGGER_TRACE(::Lemon::Log::Instance().GetCoreLogger(), __VA_ARGS__)
// Core log macros

#define LM_FATAL(...)     SPDLOG_LOGGER_CRITICAL(::Lemon::Log::Instance().GetClientLogger(), __VA_ARGS__)
#define LM_ERROR(...)     SPDLOG_LOGGER_ERROR(::Lemon::Log::Instance().GetClientLogger(), __VA_ARGS__)
#define LM_WARN(...)      SPDLOG_LOGGER_WARN(::Lemon::Log::Instance().GetClientLogger(), __VA_ARGS__)
#define LM_INFO(...)      SPDLOG_LOGGER_INFO(::Lemon::Log::Instance().GetClientLogger(), __VA_ARGS__)
#define LM_TRACE(...)     SPDLOG_LOGGER_TRACE(::Lemon::Log::Instance().GetClientLogger(), __VA_ARGS__)