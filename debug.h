#ifndef DEBUG_H
#define DEBUG_H

#include "defines.h"

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <strsafe.h>

void MakeErrorMessage(
	__in LPCTSTR lpzsFormat,
	__in LPCTSTR lpszFunction,
	__in DWORD dwErrorCode,
	__out LPTSTR *lpDisplayBuf,
	...)
{
	LPVOID lpMsgBuf;
	va_list Args;

	va_start(Args, lpDisplayBuf);

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwErrorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0,
		&Args);

	*lpDisplayBuf = (LPTSTR)LocalAlloc(
		LMEM_ZEROINIT,
		(lstrlen((LPCTSTR) lpMsgBuf) + lstrlen((LPCTSTR) lpszFunction) + 40) * sizeof(TCHAR));

	StringCchPrintf(*lpDisplayBuf,
		LocalSize(*lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dwErrorCode, lpMsgBuf);

	LocalFree(lpMsgBuf);

	va_end(Args);
}

void ErrorHandler(LPCTSTR lpszFunction)
{
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	MakeErrorMessage(
		TEXT("%s failed with error %d: %s"),
		lpszFunction,
		dw,
		(LPTSTR *) &lpDisplayBuf);

	MessageBox(
		NULL,
		(LPCTSTR) lpDisplayBuf,
		TEXT("Error"),
		MB_OK);

	// Free error-handling buffer allocations.
	LocalFree(lpDisplayBuf);
}

void TrueOrExit(LPCTSTR message, BOOL call)
{
	if(!(call))
	{
		ErrorHandler(message);
		ExitProcess(1);
	}
}

HANDLE HandleOrExit(LPCTSTR message, HANDLE call)
{
	TrueOrExit(message, call != INVALID_HANDLE_VALUE);
	return call;
}

#endif // DEBUG_H

