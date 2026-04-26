#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <array>

#include <optional>

#include "Lemon/Log.h"

#ifdef LM_PLATFORM_WINDOWS
    #define NOMINMAX
	#include <Windows.h>
    #include <d3d12.h>
    #include <dxgi.h>
#endif

