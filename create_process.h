#ifndef CREATE_PROCESS_H
#define CREATE_PROCESS_H

#include "common_compile_defines.h"

#include <Windows.h>
#include "debug.h"

void MyCreateProcess(_TCHAR *cmd, PPROCESS_INFORMATION pProcessInformation)
{
	STARTUPINFO startUpInfo;
	UINT uErrorMode;
	HANDLE hCurrentProcess, hCurrentProcessToken, hRestrictedToken;

	hCurrentProcess = GetCurrentProcess();
	TrueOrExit(OpenProcessToken(
		hCurrentProcess,
		TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_PRIVILEGES, &hCurrentProcessToken));

	TrueOrExit(CreateRestrictedToken(
		hCurrentProcessToken,
		DISABLE_MAX_PRIVILEGE,
		0, NULL,
		0, NULL,
		0, NULL,
		&hRestrictedToken));

	TrueOrExit(AdjustTokenPrivileges(hRestrictedToken, TRUE, NULL, 0, NULL, 0));

	ZeroMemory(&startUpInfo, sizeof(startUpInfo));
	startUpInfo.cb = sizeof(startUpInfo);
	ZeroMemory(pProcessInformation, sizeof(pProcessInformation));

	// Only way to change error mode of child process that I found
	// is to change parent's one, let child ingerit it
	// and restore parent's error mode back.
	uErrorMode = SetErrorMode(
		SetErrorMode(0) | SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

	TrueOrExit(CreateProcessAsUser(
		hRestrictedToken,
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

#endif // CREATE_PROCESS_H