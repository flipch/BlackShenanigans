// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"


uint64_t AddressOfDoString = -1, AddressOfGetTop = -1, jmpBackAddy = -1;

uint64_t doStringTramp = NULL, getTopTramp = NULL;

lua_State *g_lua_State;

typedef INT(CALLBACK * _DoString)(lua_State *L, CONST CHAR *s, size_t size);
_DoString oDoString;

bool Hook(void * toHook, void * ourFunct, int len) {
	if (len < 5) {
		return false;
	}

	DWORD curProtection;
	VirtualProtect(toHook, len, PAGE_EXECUTE_READWRITE, &curProtection);

	memset(toHook, 0x90, len);

	DWORD64 relativeAddress = ((DWORD64)ourFunct - (DWORD64)toHook) - 5;

	*(BYTE*)toHook = 0xE9;
	*(DWORD64*)((DWORD64)toHook + 1) = relativeAddress;

	DWORD temp;
	VirtualProtect(toHook, len, curProtection, &temp);

	return true;
}

BOOL hook_function(PVOID & t1, PBYTE t2, const char * s = NULL)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&t1, t2);
	if (DetourTransactionCommit() != NO_ERROR) {

		printf("[Hook] : Failed to hook %s.\n", s);

		return false;
	}
	else {
		printf("[Hook] : Successfully hooked %s.\n", s);

		return true;
	}
}

INT(CALLBACK h_Dostring)(lua_State *L, CONST CHAR *s, size_t size)
{
	// Log lua functions.
	if (!(s[0] == '\0'))
	{
		printf("[LUA] : Exec -> %s\n", s);
	}

	// Return to original function.
	return oDoString(L, s, size);
}

typedef INT(CALLBACK * _GetTop)(lua_State *L);
_GetTop oGetTop;

INT(CALLBACK h_GetTop)(lua_State *L)
{
	// Update lua_State pointer.
	if (g_lua_State != L) {
		printf("[LUA] : Lua State changed 0x%016llX -> 0x%016llX\n", g_lua_State, L);
		g_lua_State = L;
	}
	return oGetTop(L);
}

BOOL APIENTRY DllMain(HMODULE hModule,
	uint64_t  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		// We will use signature scanning to find the function that we want to hook
		// we will find the function in IDA pro and create a signature from it:

		SigScanner _Scanner;
		Global _Global;
		Console _Console = new Console(true);
		printf("[INFO] Injected!\n");

		// Scan for signatures
		printf("[INFO] Looking for addresses:\n");
		AddressOfDoString = _Scanner.Scan(_Global.doString);
		AddressOfGetTop = _Scanner.Scan(_Global.getTop);

		// Hooking stuff
		oDoString = (_DoString)(AddressOfDoString);
		hook_function((PVOID &)oDoString, (PBYTE)h_Dostring, _Global.doString[2].c_str());

		oGetTop = (_GetTop)(AddressOfGetTop);
		hook_function((PVOID &)oGetTop, (PBYTE)h_GetTop, _Global.getTop[2].c_str());

		// Testing stuff.
		while (true) {
			if (GetAsyncKeyState(VK_NUMPAD5)) {
				std::string cmd = "Inventory_SlotLClick(1)";
				oDoString(g_lua_State, cmd.c_str(), cmd.size());
				Sleep(150);
				cmd = "Inventory_SlotDrag(1)";
				oDoString(g_lua_State, cmd.c_str(), cmd.size());
				Sleep(150);
				cmd = "Inventory_SlotLClick(2)";
				oDoString(g_lua_State, cmd.c_str(), cmd.size());
				Sleep(150);
				cmd = "Inventory_DropHandler(2)";
				oDoString(g_lua_State, cmd.c_str(), cmd.size());
				Sleep(1000);
			}
		}

		return TRUE;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		return TRUE;
	}
}

