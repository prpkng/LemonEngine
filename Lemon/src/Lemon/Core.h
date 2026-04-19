#pragma once

#ifdef LM_PLATFORM_WINDOWS
	#ifdef LM_BUILD_DLL
		#define LEMON_API __declspec(dllexport)
	#else
		#define LEMON_API __declspec(dllimport)
	#endif // LM_BUILD_DLL
#else
	#error Lemon only support Windows!
#endif // LM_PLATFORM_WINDOWS

#define BIT(x) (1 << x)

#ifdef LM_ENABLE_ASSERTS
	#define LM_ASSERT(x, ...) { if(!(x)) { LM_ERROR("Assertion Failed: {0}", fmt::format(__VA_ARGS__)); __debugbreak(); } }
	#ifdef LM_BUILD_DLL
		#define LM_CORE_ASSERT(x, ...) { if(!(x)) { LM_CORE_ERROR("Assertion Failed: {0}", fmt::format(__VA_ARGS__)); __debugbreak(); } }
	#endif
#else
	#define LM_ASSERT(x, ...)
	#define LM_CORE_ASSERT(x, ...)
#endif