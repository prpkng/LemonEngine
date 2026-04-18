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