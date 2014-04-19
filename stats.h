#ifndef STATS_H
#define STATS_H

#include "common_compile_defines.h"

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <strsafe.h>
#include "debug.h"

#define TICKS_PER_MILLISECOND 10000

typedef struct _STATS {
	DWORD dwTimeMilliseconds;
	SIZE_T MemoryBytes;
} STATS, *PSTATS;


void GetTimeStatsOld(HANDLE hProcess, PSTATS pStats)
{
	FILETIME ftCreationTime, ftExitTime, ftKernelTime, ftUserTime;
	ULARGE_INTEGER uliKernelTime, uliUserTime;

	GetProcessTimes(
		hProcess,
		&ftCreationTime,
		&ftExitTime,
		&ftKernelTime,
		&ftUserTime);
	uliKernelTime.HighPart = ftKernelTime.dwHighDateTime;
	uliKernelTime.LowPart = ftKernelTime.dwLowDateTime;
	uliUserTime.HighPart = ftUserTime.dwHighDateTime;
	uliUserTime.LowPart = ftUserTime.dwLowDateTime;

	// Have you ever seen such accurate rounding up?
	pStats->dwTimeMilliseconds = (DWORD)((uliUserTime.QuadPart + TICKS_PER_MILLISECOND - 1) / TICKS_PER_MILLISECOND);
}

void GetTimeStats(HANDLE hJob, PSTATS pStats)
{
	JOBOBJECT_BASIC_ACCOUNTING_INFORMATION BasicAccountingInfo;

	QueryInformationJobObject(
		hJob,
		JobObjectBasicAccountingInformation,
		&BasicAccountingInfo,
		sizeof(BasicAccountingInfo),
		NULL);

	// Have you ever seen such accurate rounding up?
	pStats->dwTimeMilliseconds = (DWORD)((BasicAccountingInfo.TotalUserTime.QuadPart + TICKS_PER_MILLISECOND - 1) / TICKS_PER_MILLISECOND);
}


void GetMemoryStats(HANDLE hJob, PSTATS pStats)
{
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION ExtendedLimitInfo;

	QueryInformationJobObject(
		hJob,
		JobObjectExtendedLimitInformation,
		&ExtendedLimitInfo,
		sizeof(ExtendedLimitInfo),
		NULL);

	pStats->MemoryBytes = ExtendedLimitInfo.PeakJobMemoryUsed;
}

void GetStats(HANDLE hJob, HANDLE hProcess, PSTATS pStats)
{
	GetTimeStats(hJob, pStats);
	GetMemoryStats(hJob, pStats);
}

#endif // STATS_H