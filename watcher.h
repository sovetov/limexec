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
			pWatcherThreadData->hJob,
			pWatcherThreadData->dwTimeLimitMilliseconds))
		{
			_ftprintf(stderr, TEXT("Executor. Time limit exceeded\n"));
			fflush(stderr);
			PostQueuedCompletionStatus(
				pWatcherThreadData->hCompletionPort,
				CUSTOM_JOB_OBJECT_MSG_END_OF_PROCESS_TIME,
				0,
				NULL);
			_ftprintf(
				stderr,
				TEXT("Executor. Watcher thread. PostQueuedCompletionStatus has just been called with message %u\n"),
				CUSTOM_JOB_OBJECT_MSG_END_OF_PROCESS_TIME);
			fflush(stderr);
			break;
		}

		if(CheckMemoryLimitExceeded(
			pWatcherThreadData->hJob,
			pWatcherThreadData->MemoryLimitBytes))
		{
			_ftprintf(stderr, TEXT("Executor. Watcher thread. Memory limit exceeded\n"));
			fflush(stderr);
			PostQueuedCompletionStatus(
				pWatcherThreadData->hCompletionPort,
				CUSTOM_JOB_OBJECT_MSG_JOB_MEMORY_LIMIT,
				0,
				NULL);
			_ftprintf(
				stderr,
				TEXT("Executor. Watcher thread. PostQueuedCompletionStatus has just been called with message %u\n"),
				CUSTOM_JOB_OBJECT_MSG_JOB_MEMORY_LIMIT);
			fflush(stderr);
			break;
		}

		if(dwIterationsLmit == 0)
		{
			_ftprintf(stderr, TEXT("Executor. Watcher thread. Idleness limit exceeded\n"));
			fflush(stderr);
			PostQueuedCompletionStatus(
				pWatcherThreadData->hCompletionPort,
				CUSTOM_JOB_OBJECT_MSG_END_OF_IDLENESS,
				0,
				NULL);
			_ftprintf(
				stderr,
				TEXT("Executor. Watcher thread. PostQueuedCompletionStatus has just been called with message %u\n"),
				CUSTOM_JOB_OBJECT_MSG_END_OF_IDLENESS);
			fflush(stderr);
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