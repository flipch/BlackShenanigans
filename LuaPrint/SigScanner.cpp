#include "stdafx.h"
#include "SigScanner.h"


SigScanner::SigScanner()
{
}

SigScanner::~SigScanner()
{
}

uint64_t SigScanner::Scan(std::vector<std::string> sig)
{
	uint64_t address = -1;

	// Scanning.
	address = FindPattern(NULL, sig[0].c_str(), sig[1].c_str());

	// Output.
	if (address == -1) {
		std::cout << "[SIG] : Failed at scanning " << sig[2] << std::endl;
	}
	else {
		std::cout << "[SIG] : Succeeded at scanning " << sig[2] << std::endl << "[SIG] : Address -> 0x" << std::hex << address << std::endl;
	}
	return address;
}

uint64_t SigScanner::FindPattern(char * module, const char * pattern, const char * mask)
{
	MODULEINFO mInfo = GetModuleInfo(module);
	uint64_t base = (uint64_t)mInfo.lpBaseOfDll;
	uint64_t size = (uint64_t)mInfo.SizeOfImage;
	uint64_t patternLength = (uint64_t)strlen(mask);

	for (uint64_t i = 0; i < size - patternLength; i++)
	{
		bool found = true;
		for (uint64_t j = 0; j < patternLength; j++)
		{
			found &= mask[j] == '?' || pattern[j] == *(char*)(base + i + j);
		}
		if (found)
		{
			return base + i;
		}
	}
	return NULL;
}

MODULEINFO SigScanner::GetModuleInfo(char * szModule)
{
	MODULEINFO modinfo = { 0 };
	HMODULE hModule = GetModuleHandle(szModule);
	if (hModule == 0)
		return modinfo;
	GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
	return modinfo;
}
