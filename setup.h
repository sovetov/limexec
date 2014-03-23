#ifndef SETUP_H
#define SETUP_H

#include "common_compile_defines.h"

#include <Windows.h>

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

void MySetBreakawayLimit(HANDLE hJob)
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

void MySetBasicUIRestrictions(HANDLE hJob)
{
	JOBOBJECT_BASIC_UI_RESTRICTIONS BasicUIRestrictions;

	ZeroMemory(
		&BasicUIRestrictions,
		sizeof(BasicUIRestrictions));
	QueryInformationJobObject(
		hJob,
		JobObjectBasicUIRestrictions,
		&BasicUIRestrictions,
		sizeof(BasicUIRestrictions), NULL);
	BasicUIRestrictions.UIRestrictionsClass = JOB_OBJECT_UILIMIT_ALL;
	TrueOrExit(SetInformationJobObject(
		hJob,
		JobObjectBasicUIRestrictions,
		&BasicUIRestrictions,
		sizeof(BasicUIRestrictions)));
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

#endif // SETUP_H