#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include <Windows.h>
#include "debug.h"
#include "memory_and_time.h"

#define RUN_RESULT_SUCCESS 0
#define RUN_RESULT_RUNTIME_ERROR 1
#define RUN_RESULT_TIME_LIMIT_EXCEEDED 2
#define RUN_RESULT_MEMORY_LIMIT_EXCEEDED 3

void MyAssociateCompletionPort(HANDLE hJob, HANDLE hCompletionPort)
{
	JOBOBJECT_ASSOCIATE_COMPLETION_PORT associateCompletionPort;

	ZeroMemory(&associateCompletionPort, sizeof(associateCompletionPort));
	QueryInformationJobObject(
		hJob,
		JobObjectAssociateCompletionPortInformation,
		&associateCompletionPort,
		sizeof(associateCompletionPort),
		NULL);
	associateCompletionPort.CompletionKey = NULL;
	associateCompletionPort.CompletionPort = hCompletionPort;
	TrueOrExit(SetInformationJobObject(
		hJob,
		JobObjectAssociateCompletionPortInformation,
		&associateCompletionPort,
		sizeof(associateCompletionPort)));
}

void MySetBreakawayLimit(HANDLE hJob, LONGLONG TimeLimitMs)
{
	JOBOBJECT_BASIC_LIMIT_INFORMATION basicLimitInformationForTimeLimit;

	ZeroMemory(
		&basicLimitInformationForTimeLimit,
		sizeof(basicLimitInformationForTimeLimit));
	QueryInformationJobObject(
		hJob,
		JobObjectBasicLimitInformation,
		&basicLimitInformationForTimeLimit,
		sizeof(basicLimitInformationForTimeLimit),
		NULL);
	basicLimitInformationForTimeLimit.LimitFlags |= JOB_OBJECT_LIMIT_BREAKAWAY_OK;
	TrueOrExit(SetInformationJobObject(
		hJob,
		JobObjectBasicLimitInformation,
		&basicLimitInformationForTimeLimit,
		sizeof(basicLimitInformationForTimeLimit)));
}

void MySetTimeLimit(HANDLE hJob, LONGLONG TimeLimitMs)
{
	JOBOBJECT_BASIC_LIMIT_INFORMATION basicLimitInformationForTimeLimit;

	ZeroMemory(
		&basicLimitInformationForTimeLimit,
		sizeof(basicLimitInformationForTimeLimit));
	QueryInformationJobObject(
		hJob,
		JobObjectBasicLimitInformation,
		&basicLimitInformationForTimeLimit,
		sizeof(basicLimitInformationForTimeLimit), NULL);
	basicLimitInformationForTimeLimit.LimitFlags |= JOB_OBJECT_LIMIT_JOB_TIME;
	basicLimitInformationForTimeLimit.PerJobUserTimeLimit.QuadPart = TimeLimitMs * 10000;
	TrueOrExit(SetInformationJobObject(
		hJob,
		JobObjectBasicLimitInformation,
		&basicLimitInformationForTimeLimit,
		sizeof(basicLimitInformationForTimeLimit)));
}

void MySetMemoryLimit(HANDLE hJob, SIZE_T JobLimitBytes)
{
	JOBOBJECT_EXTENDED_LIMIT_INFORMATION extendedLimitInformationForMemoryLimit;

	ZeroMemory(
		&extendedLimitInformationForMemoryLimit,
		sizeof(extendedLimitInformationForMemoryLimit));
	QueryInformationJobObject(
		hJob,
		JobObjectExtendedLimitInformation,
		&extendedLimitInformationForMemoryLimit,
		sizeof(extendedLimitInformationForMemoryLimit),
		NULL);
	extendedLimitInformationForMemoryLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
	extendedLimitInformationForMemoryLimit.JobMemoryLimit = JobLimitBytes;
	TrueOrExit(SetInformationJobObject(
		hJob,
		JobObjectExtendedLimitInformation,
		&extendedLimitInformationForMemoryLimit,
		sizeof(extendedLimitInformationForMemoryLimit)));
}

void MySetEndOfJobTimeInformation(HANDLE hJob)
{
	JOBOBJECT_END_OF_JOB_TIME_INFORMATION endOfJobTimeInformation;

	ZeroMemory(
		&endOfJobTimeInformation,
		sizeof(endOfJobTimeInformation));
	QueryInformationJobObject(
		hJob,
		JobObjectEndOfJobTimeInformation,
		&endOfJobTimeInformation,
		sizeof(endOfJobTimeInformation), NULL);
	endOfJobTimeInformation.EndOfJobTimeAction = JOB_OBJECT_POST_AT_END_OF_JOB;
	TrueOrExit(SetInformationJobObject(
		hJob,
		JobObjectEndOfJobTimeInformation,
		&endOfJobTimeInformation,
		sizeof(endOfJobTimeInformation)));
}

void MyCreateProcess(_TCHAR *cmd, PPROCESS_INFORMATION pProcessInformation)
{
	STARTUPINFO startUpInfo;
	UINT uErrorMode;

	ZeroMemory(&startUpInfo, sizeof(startUpInfo));
	startUpInfo.cb = sizeof(startUpInfo);
	ZeroMemory(pProcessInformation, sizeof(pProcessInformation));

	// Only way to change error mode of child process that I found
	// is to change parent's one, let child ingerit it
	// and restore parent's error mode back.
	uErrorMode = SetErrorMode(
		SetErrorMode(0) | SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

	TrueOrExit(CreateProcess(
		NULL,
		cmd,
		NULL,
		NULL,
		FALSE,
		CREATE_BREAKAWAY_FROM_JOB | CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&startUpInfo,
		pProcessInformation));

	SetErrorMode(uErrorMode);
}

int DispatchMessageIdentifier(
	DWORD dwNumberOfBytesAsMessageIdentifier,
	LPOVERLAPPED lpOverlapped,
	HANDLE hProcess)
{
	DWORD dwExitCode;

	switch(dwNumberOfBytesAsMessageIdentifier)
	{
	case JOB_OBJECT_MSG_EXIT_PROCESS:
		TrueOrExit(GetExitCodeProcess(hProcess, &dwExitCode));
		return dwExitCode == 0 ? RUN_RESULT_SUCCESS : RUN_RESULT_RUNTIME_ERROR;
	case JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS:
		return RUN_RESULT_RUNTIME_ERROR;
	case JOB_OBJECT_MSG_END_OF_JOB_TIME:
	case JOB_OBJECT_MSG_END_OF_PROCESS_TIME:
		return RUN_RESULT_TIME_LIMIT_EXCEEDED;
	case JOB_OBJECT_MSG_JOB_MEMORY_LIMIT:
	case JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT:
		return RUN_RESULT_MEMORY_LIMIT_EXCEEDED;
	}

	assert(FALSE);
}

int _tmain(int argc, _TCHAR *argv[])
{
	HANDLE hJob;
	PROCESS_INFORMATION ProcessInformation;
	HANDLE hPeakMemoryThread;
	DWORD dwPeakMemoryThreadId;
	PEAK_MEMORY_USAGE_DATA MemoryAndTimeThreadData;
	DWORD dwNumberOfBytes;
	HANDLE hCompletionPort;
	ULONG CompletionKey;
	LPOVERLAPPED lpOverlapped;
	LONGLONG TimeLimitMs;
	SIZE_T MemoryLimitBytes;
	_TCHAR *CommandLine;
	int runResult;

	assert(argc >= 4);
	CommandLine = argv[1];
	TimeLimitMs = _ttoi(argv[2]);
	MemoryLimitBytes = _ttoi(argv[3]);

	hJob = HandleOrExit(CreateJobObject(NULL, NULL));
	MySetTimeLimit(hJob, TimeLimitMs);
	MySetMemoryLimit(hJob, MemoryLimitBytes);
	hCompletionPort = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		NULL,
		JOB_OBJECT_MSG_END_OF_PROCESS_TIME,
		1);
	MyAssociateCompletionPort(hJob, hCompletionPort);
	MySetEndOfJobTimeInformation(hJob);
	MyCreateProcess(CommandLine, &ProcessInformation);
	TrueOrExit(AssignProcessToJobObject(
		hJob,
		ProcessInformation.hProcess));

	GetQueuedCompletionStatus(
		hCompletionPort,
		&dwNumberOfBytes,
		&CompletionKey,
		&lpOverlapped,
		0);
	assert(dwNumberOfBytes == JOB_OBJECT_MSG_NEW_PROCESS);

	MemoryAndTimeThreadData.hJob = hJob;
	MemoryAndTimeThreadData.hProcess = ProcessInformation.hProcess;
	hPeakMemoryThread = HandleOrExit(CreateThread(
		NULL,
		0,
		MemoryAndTimeThread,
		&MemoryAndTimeThreadData,
		0,
		&dwPeakMemoryThreadId));

	GetQueuedCompletionStatus(
		hCompletionPort,
		&dwNumberOfBytes,
		&CompletionKey,
		&lpOverlapped,
		INFINITE);
	runResult = DispatchMessageIdentifier(
		dwNumberOfBytes,
		lpOverlapped,
		ProcessInformation.hProcess);

	TerminateProcess(ProcessInformation.hProcess, -1);

	// WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
	CloseHandle(ProcessInformation.hProcess);
	CloseHandle(ProcessInformation.hThread);

	return runResult;
}