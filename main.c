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

#define UPDATE_INTERVAL 100

int _tmain(int argc, _TCHAR *argv[])
{
	HANDLE hJob;
	PROCESS_INFORMATION ProcessInformation;
	HANDLE hWatcherThread;
	DWORD dwWatcherThreadId;
	WATCHER_DATA WatcherThreadData;
	DWORD dwNumberOfBytes;
	HANDLE hCompletionPort;
	ULONG CompletionKey;
	LPOVERLAPPED lpOverlapped;
	DWORD dwTimeLimitMilliseconds;
	SIZE_T MemoryLimitBytes;
	_TCHAR *CommandLine;
	VERDICT Verdict;
	STATS Stats;

	if(argc != 4)
	{
		Output(TEXT("Usage %s <command_line> <time_limit_ms> <memory_limit_bytes>"));
		ExitProcess(1);
	}

	CommandLine = argv[1];
	dwTimeLimitMilliseconds = _ttoi(argv[2]);
	MemoryLimitBytes = _ttoi(argv[3]);

	hJob = HandleOrExit(CreateJobObject(NULL, NULL));
	// MySetBreakawayLimit(hJob);
	MySetBasicUIRestrictions(hJob);

	MySetTimeLimit(hJob, dwTimeLimitMilliseconds);
	MySetMemoryLimit(hJob, MemoryLimitBytes);

	hCompletionPort = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		INVALID_HANDLE_VALUE,
		JOB_OBJECT_MSG_END_OF_PROCESS_TIME,
		1);

	MyAssociateCompletionPort(hJob, hCompletionPort);
	MySetEndOfJobTimeInformation(hJob);

	MyCreateProcess(CommandLine, &ProcessInformation);
	TrueOrExit(AssignProcessToJobObject(
		hJob,
		ProcessInformation.hProcess));

	// Here we immediately get message on completion port.
	// Note that wait time is 0.
	// This is a mew process message.
	// Also, we perform extra assert.
	// Number of bytes indicates which job-related event occurred.
	// For more information, see the following table of message identifiers.
	// See http://msdn.microsoft.com/en-us/library/windows/desktop/ms684141(v=vs.85).aspx
	GetQueuedCompletionStatus(
		hCompletionPort,
		&dwNumberOfBytes,
		&CompletionKey,
		&lpOverlapped,
		0);
	assert(dwNumberOfBytes == JOB_OBJECT_MSG_NEW_PROCESS);

	WatcherThreadData.hJob = hJob;
	WatcherThreadData.hProcess = ProcessInformation.hProcess;
	WatcherThreadData.dwTimeLimitMilliseconds = dwTimeLimitMilliseconds;
	WatcherThreadData.MemoryLimitBytes = MemoryLimitBytes;
	WatcherThreadData.dwUpdateIntervalMilliseconds = UPDATE_INTERVAL;
	WatcherThreadData.hCompletionPort = hCompletionPort;
	hWatcherThread = HandleOrExit(CreateThread(
		NULL,
		0,
		WatcherThread,
		&WatcherThreadData,
		0,
		&dwWatcherThreadId));

	// This function waits for process for a long time until it ends.
	// Number of bytes indicates which job-related event occurred.
	// For more information, see the following table of message identifiers.
	// See http://msdn.microsoft.com/en-us/library/windows/desktop/ms684141(v=vs.85).aspx
	GetQueuedCompletionStatus(
		hCompletionPort,
		&dwNumberOfBytes,
		&CompletionKey,
		&lpOverlapped,
		INFINITE);

	// TerminateProcess is asynchronous; it initiates termination and returns immediately.
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms686714(v=vs.85).aspx
	TerminateProcess(
		ProcessInformation.hProcess,
		CUSTOM_STATUS_TERMINATED_BY_CHECKER);
	WaitForSingleObject(
		ProcessInformation.hProcess,
		INFINITE);

	DispatchQueuedCompletionStatusToVerdict(
		dwNumberOfBytes,
		ProcessInformation.hProcess,
		&Verdict);
	GetStats(hJob, ProcessInformation.hProcess, &Stats);

	Output(
		TEXT("%d %d %d %s\n"),
		Verdict.verdictCode,
		Stats.dwTimeMilliseconds,
		Stats.MemoryBytes,
		Verdict.exitCodeMessage);

	// WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
	CloseHandle(ProcessInformation.hProcess);
	CloseHandle(ProcessInformation.hThread);
}