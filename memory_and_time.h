#ifndef MEMORY_AND_TIME_H
#define MEMORY_AND_TIME_H

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <strsafe.h>
#include "debug.h"
#include "output.h"

#define UPDATE_INTERVAL 500

typedef struct {
	HANDLE hJob;
	HANDLE hProcess;
} PEAK_MEMORY_USAGE_DATA, *PPEAK_MEMORY_USAGE_DATA;

void MemoryAndTime(HANDLE hJob, HANDLE hProcess)
{
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION ExtendedLimitInfo;
	FILETIME ftCreationTime, ftExitTime, ftKernelTime, ftUserTime;
	ULARGE_INTEGER uliKernelTime, uliUserTime;

	QueryInformationJobObject(
		hJob,
		JobObjectExtendedLimitInformation,
		&ExtendedLimitInfo,
		sizeof(ExtendedLimitInfo),
		NULL);
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
	Output(
		TEXT("\rProcess memory: %.2f MB, job memory: %.2f MB, process kernel time: %.2f, process user time: %.2f"),
		(double)ExtendedLimitInfo.PeakProcessMemoryUsed / 1024 / 1024,
		(double)ExtendedLimitInfo.PeakJobMemoryUsed / 1024 / 1024,
		(double)uliKernelTime.QuadPart / 10000,
		(double)uliUserTime.QuadPart / 10000);
	Sleep(UPDATE_INTERVAL);
}

DWORD WINAPI MemoryAndTimeThread(LPVOID lpParam)
{
	PPEAK_MEMORY_USAGE_DATA pMemoryAndTimeThreadData;

	pMemoryAndTimeThreadData = (PPEAK_MEMORY_USAGE_DATA)lpParam;

	// Print the parameter values using thread-safe functions.
	while(TRUE)
	{
		MemoryAndTime(pMemoryAndTimeThreadData->hJob, pMemoryAndTimeThreadData->hProcess);
	}

	return 0;
}

#endif // MEMORY_AND_TIME_H