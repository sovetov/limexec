#ifndef CREATE_PROCESS_H
#define CREATE_PROCESS_H

#include "common_compile_defines.h"

#include <Windows.h>
#include "debug.h"

void MyCreateProcess(_TCHAR *cmd, PPROCESS_INFORMATION pProcessInformation, int redirectStreams)
{
	STARTUPINFO startUpInfo;
	UINT uErrorMode;
	HANDLE hCurrentProcess, hCurrentProcessToken, hRestrictedToken;
	DWORD dwCwdBufLen = 500 * sizeof(TCHAR);
	LPTSTR lpCwdBuf = (LPTSTR)LocalAlloc(LMEM_ZEROINIT, dwCwdBufLen);

	GetCurrentDirectory(dwCwdBufLen, lpCwdBuf);

	hCurrentProcess = GetCurrentProcess();
	TrueOrExit(TEXT("OpenProcessToken"), OpenProcessToken(
		hCurrentProcess,
		TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_PRIVILEGES, &hCurrentProcessToken));

	TrueOrExit(TEXT("CreateRestrictedToken"), CreateRestrictedToken(
		hCurrentProcessToken,
		DISABLE_MAX_PRIVILEGE,
		0, NULL,
		0, NULL,
		0, NULL,
		&hRestrictedToken));

	TrueOrExit(TEXT("AdjustTokenPrivileges"), AdjustTokenPrivileges(hRestrictedToken, TRUE, NULL, 0, NULL, 0));

	ZeroMemory(&startUpInfo, sizeof(startUpInfo));
	startUpInfo.cb = sizeof(startUpInfo);
	startUpInfo.dwFlags |= STARTF_USESTDHANDLES;
	if(redirectStreams)
	{
		startUpInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
		startUpInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		startUpInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	}
	else
	{
		startUpInfo.hStdInput = INVALID_HANDLE_VALUE;
		startUpInfo.hStdOutput = INVALID_HANDLE_VALUE;
		startUpInfo.hStdError = INVALID_HANDLE_VALUE;
	}
	startUpInfo.dwFlags |= STARTF_USESHOWWINDOW;
	startUpInfo.wShowWindow = SW_HIDE;
	ZeroMemory(pProcessInformation, sizeof(*pProcessInformation));

	// Only way to change error mode of child process that I found
	// is to change parent's one, let child inherit it
	// and restore parent's error mode back.
	uErrorMode = SetErrorMode(
		SetErrorMode(0) | SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

	TrueOrExit(TEXT("CreateProcessAsUser"), CreateProcessAsUser(
		hRestrictedToken,
		NULL,
		cmd,
		NULL,
		NULL,
		TRUE,
		CREATE_NO_WINDOW | CREATE_BREAKAWAY_FROM_JOB,
		NULL,
		lpCwdBuf,
		&startUpInfo,
		pProcessInformation));

	/*
	TrueOrExit(TEXT("CreateProcess"), CreateProcess(
		NULL,
		cmd,
		NULL,
		NULL,
		FALSE,
		CREATE_NO_WINDOW | CREATE_BREAKAWAY_FROM_JOB,
		NULL,
		lpCwdBuf,
		&startUpInfo,
		pProcessInformation));
		*/

	LocalFree(lpCwdBuf);
	SetErrorMode(uErrorMode);
}

#endif // CREATE_PROCESS_H