#include "stdafx.h"
#include "Hooks.h"
#include "Global.h"


Hooks::Hooks(Global g)
{
	_Global = g;
	oDoString = (_DoString)(_Global.AddressDoString);/*
	hook_function((PVOID &)oDoString, (PBYTE)h_Dostring, _Global.doString[2].c_str());

	oGetTop = (_GetTop)(_Global.AddressGetTop);
	hook_function((PVOID &)oGetTop, (PBYTE)h_GetTop, _Global.getTop[2].c_str());*/
}


Hooks::~Hooks()
{
}

bool hook_function(PVOID & t1, PBYTE t2, const char * s = NULL)
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
/*
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

INT(CALLBACK h_GetTop)(lua_State *L)
{
	// Update lua_State pointer.
	if (_Global.lua != L) {
		printf("[LUA] : Lua State changed 0x%016llX -> 0x%016llX\n", _Global.lua_state, L);
		_Global.lua_state = L;
	}
	return oGetTop(L);
}
*/