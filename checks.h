#ifndef CHECKS_H
#define CHECKS_H

#include "common_compile_defines.h"

#include <Windows.h>
#include "stats.h"

BOOL CheckTimeLimitExceeded(HANDLE hProcess, DWORD dwTimeLimitMilliseconds)
{
	STATS Stats;
	GetTimeStats(hProcess, &Stats);

	return Stats.dwTimeMilliseconds > dwTimeLimitMilliseconds;
}

BOOL CheckMemoryLimitExceeded(HANDLE hJob, SIZE_T MemoryLimitBytes)
{
	STATS Stats;
	GetMemoryStats(hJob, &Stats);

	return Stats.MemoryBytes > MemoryLimitBytes;
}

#endif // CHECKS_H