#pragma once

// Imports
#include <stdio.h>
#include <array>
#include <map>
#include <windows.h>
#include <iostream>
#include "MinHook/include/MinHook.h"
#include <initializer_list>


// Imgui 
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

// DirectX
#include <d3d11.h>
#include <d3dcompiler.h>

#ifdef _MSC_VER
#pragma comment(lib, "d3d11")
#pragma comment(lib, "winmm")
#pragma comment(lib, "d3dcompiler")
#endif

using std::cout;
using std::endl;

namespace wdi
{
	// Console logging
	//
	template<typename... T>
	constexpr auto console_log(const T& ...t) -> void // this is indeed bad practice
	{																									// but i had too after running into linker issues
		auto f = [](const auto& t) { cout << t << " "; };
		(f(t), ...);
		cout << "\n";
	}

	auto create_console() -> void;

	// Directx 11 usage
	//
	auto hPresent(
		IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags
	) -> HRESULT __fastcall;

	auto Init(void** pSwapchain_table, std::size_t size_Swapchain,
		void** pDevice_table,  std::size_t size_device,
		void** pContext_table, std::size_t size_context) -> bool;

	auto GetDeviceAndCtx(IDXGISwapChain* pSwapChain, 
		ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext) -> HRESULT;

	LRESULT CALLBACK hwnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	auto run_hook() -> void;

};