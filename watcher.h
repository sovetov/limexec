#ifndef WATCHER_H
#define WATCHER_H

#include "common_compile_defines.h"

#include <Windows.h>
#include "checks.h"
#include "custom_job_object_msgs.h"

typedef struct {
	HANDLE hJob;
	HANDLE hProcess;
	DWORD dwUpdateIntervalMilliseconds, dwTimeLimitMilliseconds;
	SIZE_T MemoryLimitBytes;
	HANDLE hCompletionPort;
} WATCHER_DATA, *PWATCHER_DATA;

DWORD WINAPI WatcherThread(LPVOID lpParam)
{
	PWATCHER_DATA pWatcherThreadData;

	pWatcherThreadData = (PWATCHER_DATA)lpParam;

	// Print the parameter values using thread-safe functions.
	while(TRUE)
	{
		if(CheckTimeLimitExceeded(
			pWatcherThreadData->hProcess,
			pWatcherThreadData->dwTimeLimitMilliseconds))
		{
			PostQueuedCompletionStatus(
				pWatcherThreadData->hCompletionPort,
				CUSTOM_JOB_OBJECT_MSG_END_OF_PROCESS_TIME,
				0,
				NULL);
			break;
		}

		if(CheckMemoryLimitExceeded(
			pWatcherThreadData->hJob,
			pWatcherThreadData->MemoryLimitBytes))
		{
			PostQueuedCompletionStatus(
				pWatcherThreadData->hCompletionPort,
				CUSTOM_JOB_OBJECT_MSG_JOB_MEMORY_LIMIT,
				0,
				NULL);
			break;
		}

		Sleep(pWatcherThreadData->dwUpdateIntervalMilliseconds);
	}

	return 0;
}

#endif // WATCHER_H