#ifndef OUTPUT_H
#define OUTPUT_H

#include "common_compile_defines.h"

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <strsafe.h>
#include <varargs.h>
#include "debug.h"

#define BUF_SIZE 255

BOOL Output(__in __format_string LPCTSTR msg, ...)
{
	HANDLE hStdout;
	TCHAR msgBuf[BUF_SIZE];
	SIZE_T cchStringSize;
	DWORD dwChars;

	va_list argList;

	// Make sure there is a console to receive output results.
	hStdout = HandleOrExit(GetStdHandle(STD_OUTPUT_HANDLE));

	va_start(argList, msg);

	StringCchVPrintf(msgBuf, BUF_SIZE, msg, argList);
	StringCchLength(msgBuf, BUF_SIZE, &cchStringSize);

	va_end(argList);

	return WriteConsole(hStdout, msgBuf, (DWORD)cchStringSize, &dwChars, NULL);
}

#endif // OUTPUT_H