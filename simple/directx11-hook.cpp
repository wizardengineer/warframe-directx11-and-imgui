#include "directx11-hook.hpp"

// Tabs helper
//

enum 
	class OptionType 
	: uint8_t
{
	Label,
	CheckBox
};

union OptionVariant
{
	int iVlaue;
	bool bValue;
};

struct Option
{
	std::string label;
	OptionType type;
	OptionVariant value;

	Option(std::string label, OptionType type, OptionVariant value) 
	: label(label), type(type), value(value) { }

	~Option() = default;
};

// Hooks
//
typedef
HRESULT(__fastcall* IDXGISwapChainPresent)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);

typedef 
LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

// Gui init variables
//
static bool g_initialized = false;
static bool g_showmenu    = false;

// D3D11 objects
//

static ID3D11DeviceContext       *p_context          = nullptr;
static ID3D11Device              *p_device           = nullptr;
static IDXGISwapChain            *p_swapchain        = nullptr;
static HWND                       window             = nullptr;
static WNDPROC                    orignal_wnd_hander = nullptr;
static IDXGISwapChainPresent           fnIDXGISwapChainPresent;
static ID3D11RenderTargetView          *main_render_targetview;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace wdi
{
	// Console logging
	//


	auto create_console() -> void
	{
		AllocConsole();
		freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
		freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
		freopen_s((FILE**)stdin, "CONOUT$", "r", stdin);
	}

	// Directx 11 implentation
	//

	auto GetDeviceAndCtx(IDXGISwapChain* pSwapChain,
		ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext) -> HRESULT
	{
		HRESULT ret = pSwapChain->GetDevice(__uuidof(ID3D11Device), (PVOID*)ppDevice);

		if (ret > 0) {
			(*ppDevice)->GetImmediateContext(ppContext);
		}

		return ret;
	}
	auto hPresent(
		IDXGISwapChain* pChain, UINT SyncInterval, UINT Flags
		) -> HRESULT __fastcall
	{
		// Testing hook
		console_log("[+] Present Hooked");

		if (!g_initialized) {
			if (FAILED(pChain->GetDevice(__uuidof(ID3D11Device), (void**)&p_device))) {
				return fnIDXGISwapChainPresent(pChain, SyncInterval, Flags);
			}


			// Setting up ImGui
			p_device->GetImmediateContext(&p_context);
			DXGI_SWAP_CHAIN_DESC sd;
			pChain->GetDesc(&sd);
			window = sd.OutputWindow;

			ID3D11Texture2D* P_backBuffer;

			pChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&P_backBuffer);
			p_device->CreateRenderTargetView(P_backBuffer, NULL, &main_render_targetview);
			P_backBuffer->Release();
			orignal_wnd_hander = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)hwnd_proc);

			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
			ImGui_ImplWin32_Init(window);
			ImGui_ImplDX11_Init(p_device, p_context);





			g_initialized = true;
		}

		if (GetAsyncKeyState(VK_INSERT) & 1) {
			g_showmenu = !g_showmenu;
		}



		if (g_showmenu) {
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();

			ImGui::StyleColorsDark();
			ImGui::NewFrame();

			ImGui::Begin("Simple menu");

			static bool aimbot = false;
      if (ImGui::Checkbox("Button", &aimbot));

		
			
			ImGui::EndFrame();
		
		

			// Assemble draw data
			ImGui::Render();

			p_context->OMGetRenderTargets(1, &main_render_targetview, nullptr);
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

		return fnIDXGISwapChainPresent(pChain, SyncInterval, Flags);
	}

	auto Init() -> bool
	{

		HMODULE check_dxgi;

		do {
			check_dxgi = GetModuleHandle(L"dxgi.dll");
			Sleep(4000);
		} while (!check_dxgi);
		Sleep(100);

		// Holds windows class attribute
		// holds information about a 
		// window
		//
		WNDCLASSEX wc{ 0 };
		wc.cbSize = sizeof(wc);
		wc.lpfnWndProc = DefWindowProc;
		wc.lpszClassName = L"Dummy Class";

		// Register a window class
		if (!RegisterClassEx(&wc)) { return false; }

		// Creates a child window, overlap or pop-up
		window = CreateWindow(wc.lpszClassName, L"", WS_DISABLED, 0, 0, 0, 0, NULL, NULL, NULL, nullptr, );

		// Swap chain description:
		// To tell DirectX what type of swap 
		// chain to create, a swap chain description must be filled
		//
		DXGI_SWAP_CHAIN_DESC swpcd{ NULL };

		swpcd.BufferCount = 1;
		swpcd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

		// This member describes the display format to use. 
		// the color palette and such
		//
		swpcd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swpcd.OutputWindow = window;
		swpcd.SampleDesc.Count = 1;
		swpcd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swpcd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swpcd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swpcd.Windowed = TRUE;

		D3D_FEATURE_LEVEL featureL;


		// Creates a device that represents the display 
		// adapter and a swap chain used for rendering
		HRESULT h_result = D3D11CreateDeviceAndSwapChain(
			nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
			0, nullptr, 0, D3D11_SDK_VERSION, &swpcd,
			&p_swapchain, &p_device, &featureL, nullptr);

		if (h_result < 0)
		{
			console_log("[-] Creating device to represent display adapter failed", GetLastError());
			DestroyWindow(swpcd.OutputWindow);
			UnregisterClass(wc.lpszClassName, GetModuleHandle(nullptr));
			return false;
		}

		DWORD_PTR* pSwapChainVtable = nullptr;
		DWORD_PTR* pContextVtable   = nullptr;

		pSwapChainVtable = reinterpret_cast<DWORD_PTR*>(p_swapchain);
		pSwapChainVtable = reinterpret_cast<DWORD_PTR*>(pSwapChainVtable[0]);

		if (MH_Initialize() != MH_OK) { return false; }
		if (MH_CreateHook((DWORD_PTR*)pSwapChainVtable[8], hPresent, reinterpret_cast<void**>(&fnIDXGISwapChainPresent)) != MH_OK) 
		{
			console_log("CreateHook failed"); 
			return false;
		}

		if (MH_EnableHook((DWORD_PTR*)pSwapChainVtable[8]) != MH_OK) 
		{
			console_log("CreateHook failed"); 
			return false; 
		}

		while (true) {
			Sleep(100);
		}

		p_swapchain->Release();
		//p_swapchain = nullptr;

		p_device->Release();
		//p_device = nullptr;

		p_context->Release();
		// p_context = nullptr;

		DestroyWindow(swpcd.OutputWindow);
		UnregisterClass(wc.lpszClassName, GetModuleHandle(nullptr));
		return true;

	}

	LRESULT CALLBACK hwnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam)) {
			return true;
		}

		return CallWindowProc(orignal_wnd_hander, hWnd, uMsg, wParam, lParam);

	}

	

	auto run_hook() -> void
	{
		if (Init() == false) {
			console_log("Init == false");
		} 
	}
};
