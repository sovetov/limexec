#include "common_compile_defines.h"

#include <tchar.h>
#include <assert.h>
#include <Windows.h>
#include "debug.h"
#include "watcher.h"
#include "setup.h"
#include "dispatcher.h"
#include "stats.h"
#include "create_process.h"

#define UPDATE_INTERVAL (100)
#define IDLENESS_LIMIT_PER_PROCESS_TIME_LIMIT_RATIO (2)

int _tmain(int argc, _TCHAR *argv[])
{
	HANDLE hJob;
	PROCESS_INFORMATION ProcessInformation;
	HANDLE hWatcherThread;
	DWORD dwWatcherThreadId;
	HANDLE hCompletionPort;
	ULONG CompletionKey;
	LPOVERLAPPED lpOverlapped;
	DWORD dwTimeLimitMilliseconds;
	SIZE_T MemoryLimitBytes;
	_TCHAR *CommandLine;
	VERDICT Verdict;
	STATS Stats;

	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

	if(argc != 5)
	{
		_ftprintf(stderr, TEXT("Executor usage \"%s\" <command_line> <time_limit_ms> <memory_limit_bytes> <redirect_streams>"), argv[0]);
		ExitProcess(0xFF000003);
	}

	{
		DWORD dwCwdBufLen = 500 * sizeof(TCHAR);
		LPTSTR lpCwdBuf = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, dwCwdBufLen);

		GetCurrentDirectory(dwCwdBufLen, lpCwdBuf);
		_ftprintf(
			stderr,
			TEXT("Executor arguments are:\n%s\n%s\n%d\n%d\n%s\n\nExecutor working dir is:\n%s\n\n"),
			argv[0],
			argv[1],
			_ttoi(argv[2]),
			_ttoi(argv[3]),
			argv[4],
			lpCwdBuf);
		LocalFree(lpCwdBuf);
		fflush(stderr);
	}

	{
		JOBOBJECT_BASIC_LIMIT_INFORMATION BasicLimitInfo;

		ZeroMemory(
			&BasicLimitInfo,
			sizeof(BasicLimitInfo));
		QueryInformationJobObject(
			NULL,
			JobObjectBasicLimitInformation,
			&BasicLimitInfo,
			sizeof(BasicLimitInfo), NULL);

		_ftprintf(stderr, TEXT("Executor basic limit flags are:\n0x%08X\n\n"), BasicLimitInfo.LimitFlags);
		fflush(stderr);
	}

	CommandLine = argv[1];
	dwTimeLimitMilliseconds = _ttoi(argv[2]);
	MemoryLimitBytes = _ttoi(argv[3]);

	hJob = HandleOrExit(TEXT("CreateJobObject"), CreateJobObject(NULL, NULL));
	// MySetBreakawayLimit(hJob);
	MySetBasicUIRestrictions(hJob);

	MySetTimeLimit(hJob, dwTimeLimitMilliseconds);
	MySetMemoryLimit(hJob, MemoryLimitBytes);

	hCompletionPort = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		NULL,
		JOB_OBJECT_MSG_END_OF_PROCESS_TIME,
		1);

	MyAssociateCompletionPort(hJob, hCompletionPort);
	MySetEndOfJobTimeInformation(hJob);

	MyCreateProcess(CommandLine, &ProcessInformation, _ttoi(argv[4]));
	TrueOrExit(TEXT("AssignProcessToJobObject"), AssignProcessToJobObject(
		hJob,
		ProcessInformation.hProcess));
	{
		// Run watcher thread

		WATCHER_DATA WatcherThreadData;
		WatcherThreadData.hJob = hJob;
		WatcherThreadData.hProcess = ProcessInformation.hProcess;
		WatcherThreadData.dwTimeLimitMilliseconds = dwTimeLimitMilliseconds;
		WatcherThreadData.dwIdlenessLimitMilliseconds = IDLENESS_LIMIT_PER_PROCESS_TIME_LIMIT_RATIO * dwTimeLimitMilliseconds;
		WatcherThreadData.MemoryLimitBytes = MemoryLimitBytes;
		WatcherThreadData.dwUpdateIntervalMilliseconds = UPDATE_INTERVAL;
		WatcherThreadData.hCompletionPort = hCompletionPort;

		_ftprintf(stderr, TEXT("Executor. Starting watcher thread...\n"));
		fflush(stderr);
		hWatcherThread = HandleOrExit(TEXT("CreateThread"), CreateThread(
			NULL,
			0,
			WatcherThread,
			&WatcherThreadData,
			0,
			&dwWatcherThreadId));
		_ftprintf(stderr, TEXT("Executor. Watcher thread has been started\n"));
		fflush(stderr);
	}

	{
		DWORD dwNumberOfBytesAsMessageIdentifier, dwPossiblyAbnormalNumberOfBytesAsMessageIdentifier = -1;
		DWORD numberOfProcesses = 0;

		// Number of bytes indicates which job-related event occurred.
		// For more information, see the following table of message identifiers.
		// See http://msdn.microsoft.com/en-us/library/windows/desktop/ms684141(v=vs.85).aspx
		while(TRUE)
		{
			BOOL success = GetQueuedCompletionStatus(
				hCompletionPort,
				&dwNumberOfBytesAsMessageIdentifier,
				&CompletionKey,
				&lpOverlapped,
				INFINITE);

			assert(succcess);

			_ftprintf(stderr, TEXT("Executor. Message with code %u received\n"), dwNumberOfBytesAsMessageIdentifier);
			fflush(stderr);

			if(dwNumberOfBytesAsMessageIdentifier != JOB_OBJECT_MSG_NEW_PROCESS && dwNumberOfBytesAsMessageIdentifier != JOB_OBJECT_MSG_EXIT_PROCESS)
			{
				dwPossiblyAbnormalNumberOfBytesAsMessageIdentifier = dwNumberOfBytesAsMessageIdentifier;
				break;
			}
		}

		// We will not wait for something as it was before when we waited effect of asynchronous TerminateProcess
		_ftprintf(stderr, TEXT("Executor. Terminate job\n"));
		fflush(stderr);
		TrueOrExit(TEXT("Executor. TerminateJobObject"), TerminateJobObject(
			hJob,
			CUSTOM_STATUS_TERMINATED_BY_CHECKER));

		DispatchQueuedCompletionStatusToVerdict(
			dwPossiblyAbnormalNumberOfBytesAsMessageIdentifier,
			ProcessInformation.hProcess,
			&Verdict);
		GetStats(hJob, ProcessInformation.hProcess, &Stats);

		_tprintf(
			TEXT("%u %u %u %u %u %s\n"),
			Verdict.verdictCode,
			dwPossiblyAbnormalNumberOfBytesAsMessageIdentifier,
			Verdict.exitCode,
			Stats.dwTimeMilliseconds,
			Stats.MemoryBytes,
			Verdict.exitCodeMessage);
		fflush(stderr);
	}

	// WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
	CloseHandle(ProcessInformation.hProcess);
	CloseHandle(ProcessInformation.hThread);
}