#pragma once
class Hooks
{
public:
	Hooks(Global g);
	~Hooks();

	Global _Global;

	typedef INT(CALLBACK * _DoString)(lua_State *L, CONST CHAR *s, size_t size);
	_DoString oDoString;

	typedef INT(CALLBACK * _GetTop)(lua_State *L);
	_GetTop oGetTop;
};

