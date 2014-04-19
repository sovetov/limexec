#ifndef WATCHER_H
#define WATCHER_H

#include "common_compile_defines.h"

#include <Windows.h>
#include "checks.h"
#include "custom_job_object_msgs.h"

typedef struct {
	HANDLE hJob;
	HANDLE hProcess;
	DWORD dwUpdateIntervalMilliseconds, dwTimeLimitMilliseconds, dwIdlenessLimitMilliseconds;
	SIZE_T MemoryLimitBytes;
	HANDLE hCompletionPort;
} WATCHER_DATA, *PWATCHER_DATA;

DWORD WINAPI WatcherThread(LPVOID lpParam)
{
	PWATCHER_DATA pWatcherThreadData = (PWATCHER_DATA)lpParam;
	DWORD dwIterationsLmit = pWatcherThreadData->dwIdlenessLimitMilliseconds / pWatcherThreadData->dwUpdateIntervalMilliseconds;

	// Print the parameter values using thread-safe functions.
	while(TRUE)
	{
		if(CheckTimeLimitExceeded(
			pWatcherThreadData->hProcess,
			pWatcherThreadData->dwTimeLimitMilliseconds))
		{
			_ftprintf(stderr, L"Time limit exceeded\n");
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
			_ftprintf(stderr, L"Memory limit exceeded\n");
			PostQueuedCompletionStatus(
				pWatcherThreadData->hCompletionPort,
				CUSTOM_JOB_OBJECT_MSG_JOB_MEMORY_LIMIT,
				0,
				NULL);
			break;
		}

		if(dwIterationsLmit == 0)
		{
			_ftprintf(stderr, L"Idleness limit exceeded\n");
			PostQueuedCompletionStatus(
				pWatcherThreadData->hCompletionPort,
				CUSTOM_JOB_OBJECT_MSG_END_OF_IDLENESS,
				0,
				NULL);
			break;
		}
		else
		{
			--dwIterationsLmit;
		}

		Sleep(pWatcherThreadData->dwUpdateIntervalMilliseconds);
	}

	return 0;
}

#endif // WATCHER_H