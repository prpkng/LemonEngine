#pragma once

#include <lmpch.h>
#include <wrl/client.h>
#include <Lemon/Renderer/RHI/Interfaces/IBuffer.h>

#define CHECK(x, msg) { HRESULT __hr__ = x; if (FAILED(__hr__)) { LM_CORE_FATAL("{0}: {1}", msg, HrToString(__hr__)); abort(); } }

using Microsoft::WRL::ComPtr;

std::string HrToString(HRESULT hr);

DXGI_FORMAT TranslateElementTypeToFormat(Lemon::RHI::ElementType type);
