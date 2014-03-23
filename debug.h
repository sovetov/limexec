#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <strsafe.h>

void ErrorHandler(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code.

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	// Display the error message.

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR) lpMsgBuf) + lstrlen((LPCTSTR) lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR) lpDisplayBuf, TEXT("Error"), MB_OK);

	// Free error-handling buffer allocations.

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

void TrueOrExit(BOOL call)
{
	_TCHAR *message = TEXT("Error: %d");

	if(!(call))
	{
		if(TRUE)
			ErrorHandler(TEXT("Fucntion"));
		else
		{
			DWORD lastError = GetLastError();
			_TCHAR output[500];
			_stprintf_s(output, _countof(output), message, lastError);
			MessageBox(NULL, output, _T("Fail"), MB_OK);
		}

		ExitProcess(1);
	}
}

HANDLE HandleOrExit(HANDLE call)
{
	TrueOrExit(call != INVALID_HANDLE_VALUE);
	return call;
}

#endif // DEBUG_H

