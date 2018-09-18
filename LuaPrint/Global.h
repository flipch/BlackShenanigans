#pragma once

class Global
{
public:
	lua_State *luaState;

	std::unordered_map<std::string, uint64_t> addresses;

	std::vector<std::string> doString =
	{ "\x48\x89\x5C\x24\x08\x48\x89\x6C\x24\x10\x48\x89\x74\x24\x18\x57\x48\x83\xEC\x20\x49\x8B\xD8\x48\x8B\xFA\x45\x33\xC0",
	"xxxxxxxxxxxxxxxxxxxxxxxxxxxxx", "DoString" };

	std::vector<std::string> getTop =
	{ "\x48\x8B\x41\x10\x48\x2B\x41\x18\x48\xC1\xF8\x04\xC3", "xxxxxxxxxxxxx", "GeTop" };
};

