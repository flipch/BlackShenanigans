#include "stdafx.h"
#include "Hooks.h"

Hooks::_DoString Hooks::oDoString = NULL;
Hooks::_GetTop Hooks::oGetTop = NULL;
Hooks::D3D11PresentHook Hooks::oD3D11Present = NULL;
Global Hooks::_Global;

bool firstTime = false;

ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;

DWORD_PTR* pSwapChainVtable = NULL;
DWORD_PTR* pDeviceContextVTable = NULL;

Hooks::Hooks(Global _g)
{
	Hooks::_Global = _g;
}

Hooks::~Hooks()
{
}

INT(CALLBACK h_Dostring)(lua_State *L, CONST CHAR *s, size_t size)
{
	// Log lua functions.
	if (!(s[0] == '\0'))
	{
		std::cout << "[LUA] : Exec ->" << std::hex << s << std::endl;
	}

	// Return to original function.
	return Hooks::oDoString(L, s, size);
}

INT(CALLBACK h_GetTop)(lua_State *L)
{
	// Update lua_State pointer.
	if (Hooks::_Global.luaState != L) {
		std::cout << "[LUA] : Lua State changed " << std::hex << Hooks::_Global.luaState << " -> " << std::hex << L << std::endl;
		Hooks::_Global.luaState = L;
	}
	return Hooks::oGetTop(L);
}

HRESULT __stdcall h_D3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!firstTime)
	{
		pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
		pDevice->GetImmediateContext(&pContext);

		firstTime = true;

		//Hooks::_Global.imguiInit = Renderer::InitIMGui(pDevice, pContext);
	}
	if (firstTime)
	{
		/*
		ID3D11Texture2D *pBackBuffer;
		pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);

		ID3D11RenderTargetView *pRTV;


		pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRTV);


		pContext->OMSetRenderTargets(1, &pRTV, NULL);

		D3D11_TEXTURE2D_DESC backBufferDesc;
		pBackBuffer->GetDesc(&backBufferDesc);

		pBackBuffer->Release();
		*/
	}
	return Hooks::oD3D11Present(pSwapChain, SyncInterval, Flags);
}

const void* Hooks::DetourFuncVTable(SIZE_T* src, const BYTE* dest, const uint64_t index)
{
	DWORD dwVirtualProtectBackup;
	SIZE_T* const indexPtr = &src[index];
	const void* origFunc = (void*)*indexPtr;
	VirtualProtect(indexPtr, sizeof(SIZE_T), PAGE_EXECUTE_READWRITE, &dwVirtualProtectBackup);
	*indexPtr = (SIZE_T)dest;
	VirtualProtect(indexPtr, sizeof(SIZE_T), dwVirtualProtectBackup, &dwVirtualProtectBackup);
	return origFunc;
}

bool Hooks::hook_function(PVOID & t1, PBYTE t2, const char * s)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&t1, t2);
	if (DetourTransactionCommit() != NO_ERROR) {
		std::cout << "[Hook] : Failed to hook " << s << std::endl;
		return false;
	}
	else {
		std::cout << "[Hook] : Successfully hooked " << s << std::endl;
		return true;
	}
}

bool Hooks::setHooks()
{
	bool result = true;

	// Hooking for game functions.
	Hooks::oDoString = (Hooks::_DoString)_Global.addresses["DoString"];
	Hooks::oGetTop = (Hooks::_GetTop)_Global.addresses["GetTop"];
	result &= Hooks::hook_function((PVOID &)oDoString, (PBYTE)h_Dostring, _Global.doString[2].c_str());
	result &= Hooks::hook_function((PVOID &)oGetTop, (PBYTE)h_GetTop, _Global.getTop[2].c_str());

	// Hooking for Directx.
	HWND hWnd = GetForegroundWindow();
	IDXGISwapChain* pSwapChain;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;//((GetWindowLong(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? FALSE : TRUE;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1
		, D3D11_SDK_VERSION, &swapChainDesc, &pSwapChain, &pDevice, NULL, &pContext)))
	{
		std::cout << "Failed to create directX device and swapchain!\n";
		return false;
	}
	else {
		pSwapChainVtable = (DWORD_PTR*)pSwapChain;
		pSwapChainVtable = (DWORD_PTR*)pSwapChainVtable[0];

		pDeviceContextVTable = (DWORD_PTR*)pContext;
		pDeviceContextVTable = (DWORD_PTR*)pDeviceContextVTable[0];

		Hooks::oD3D11Present = (Hooks::D3D11PresentHook)pSwapChainVtable[8];
		result &= Hooks::hook_function((PVOID &)oD3D11Present, (PBYTE)h_D3D11Present, "Present");
		pDevice->Release();
		pContext->Release();
		pSwapChain->Release();
		return result;
	}
}
