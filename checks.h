#ifndef CHECKS_H
#define CHECKS_H

#include "common_compile_defines.h"

#include <Windows.h>
#include "stats.h"

BOOL CheckTimeLimitExceeded(HANDLE hJob, DWORD dwTimeLimitMilliseconds)
{
	STATS Stats;
	GetTimeStats(hJob, &Stats);

	_ftprintf(
		stderr,
		TEXT("Executor. Watcher. Time consumed %u ms whereas limit is %u ms\n"),
		Stats.dwTimeMilliseconds,
		dwTimeLimitMilliseconds);

	return Stats.dwTimeMilliseconds > dwTimeLimitMilliseconds;
}

BOOL CheckMemoryLimitExceeded(HANDLE hJob, SIZE_T MemoryLimitBytes)
{
	STATS Stats;
	GetMemoryStats(hJob, &Stats);

	_ftprintf(
		stderr,
		TEXT("Executor. Watcher. Memory consumed %u bytes whereas limit is %u\n"),
		Stats.MemoryBytes,
		MemoryLimitBytes);

	return Stats.MemoryBytes > MemoryLimitBytes;
}

#endif // CHECKS_H