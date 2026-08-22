#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commdlg.h>

#include <fstream>
#include <memory>
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <assert.h>
#include <cmath>
#include <utility>
#include <algorithm>
#include <filesystem>
#include <cwctype>

#define DIRECTINPUT_VERSION 0x0800

#include <d3d11_3.h>
#include <DXGI1_3.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <dinput.h>
#include <wrl/client.h>

#include <DirectXTex/DirectXTex.h>

#include <spdlog/spdlog.h>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/ImGuizmo.h>

#include <assimp/Importer.hpp>

#include <nlohmann/json.hpp>

#include "Core/Common.h"
#include "Core/Types.h"
#include "Utilities/StringUtils.h"
#include "Utilities/Timer.h"