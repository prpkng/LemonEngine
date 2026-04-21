#pragma once

#include <lmpch.h>
#include <wrl/client.h>
#include <Lemon/Renderer/Buffers/VertexBuffer.h>

#define CHECK(x, msg) { HRESULT hr = x; if (FAILED(hr)) { LM_CORE_FATAL("{0}: {1}", msg, HrToString(hr)); abort(); } }

using Microsoft::WRL::ComPtr;

std::string HrToString(HRESULT hr);

DXGI_FORMAT TranslateElementTypeToFormat(Lemon::RHI::VertexElementType type);
