#ifndef CHECKS_H
#define CHECKS_H

#include "common_compile_defines.h"

#include <Windows.h>
#include "stats.h"

BOOL CheckTimeLimitExceeded(HANDLE hProcess, DWORD dwTimeLimitMilliseconds)
{
	STATS Stats;
	GetTimeStats(hProcess, &Stats);

	// _ftprintf(stderr, L"Time consumed %d\n", Stats.dwTimeMilliseconds);

	return Stats.dwTimeMilliseconds > dwTimeLimitMilliseconds;
}

BOOL CheckMemoryLimitExceeded(HANDLE hJob, SIZE_T MemoryLimitBytes)
{
	STATS Stats;
	GetMemoryStats(hJob, &Stats);

	// _ftprintf(stderr, L"Memory consumed %d\n", Stats.dwTimeMilliseconds);

	return Stats.MemoryBytes > MemoryLimitBytes;
}

#endif // CHECKS_H