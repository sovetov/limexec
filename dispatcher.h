#ifndef DISPATCHER_H
#define DISPATCHER_H

#define WIN32_LEAN_AND_MEAN
#define NOCOMM

#include <Windows.h>
#include "debug.h"
#include "custom_job_object_msgs.h"

typedef int VERDICT_CODE;
#define VERDICT_SUCCESS 0
#define VERDICT_RUNTIME_ERROR 1
#define VERDICT_TIME_LIMIT_EXCEEDED 2
#define VERDICT_MEMORY_LIMIT_EXCEEDED 3
#define VERDICT_UNKNOWN -1

typedef struct _VERDICT {
	VERDICT_CODE verdictCode;
	LPCTSTR exitCodeMessage;
} VERDICT, *PVERDICT;

#define CUSTOM_STATUS_SUCCESS 0
#define CUSTOM_STATUS_TERMINATED_BY_CHECKER 0xFF000001

// According to http://msdn.microsoft.com/en-us/library/windows/desktop/ms684141(v=vs.85).aspx
// Due to some difficulties with getting error messages via FormatMessage function
// as well as parameters for them, it was decided to output names of defines only.
// See:
// http://stackoverflow.com/questions/7915215/how-do-i-convert-a-win32-exception-code-to-a-string
// http://stackoverflow.com/questions/321898/how-to-get-the-name-description-of-an-exception
// http://stackoverflow.com/questions/455434/how-should-i-use-formatmessage-properly-in-c
// http://stackoverflow.com/questions/12724473/windows-c-formatmessage-with-getexceptioncode-ignores-format-message-igno
// http://support.microsoft.com/kb/259693
LPCTSTR GetExitCodeMessage(DWORD uExitCode)
{
	switch(uExitCode)
	{
	case STATUS_ACCESS_VIOLATION:
		return TEXT("STATUS_ACCESS_VIOLATION");
	case STATUS_ARRAY_BOUNDS_EXCEEDED:
		return TEXT("STATUS_ARRAY_BOUNDS_EXCEEDED");
	case STATUS_BREAKPOINT:
		return TEXT("STATUS_BREAKPOINT");
	case STATUS_CONTROL_C_EXIT:
		return TEXT("STATUS_CONTROL_C_EXIT");
	case STATUS_DATATYPE_MISALIGNMENT:
		return TEXT("STATUS_DATATYPE_MISALIGNMENT");
	case STATUS_FLOAT_DENORMAL_OPERAND:
		return TEXT("STATUS_FLOAT_DENORMAL_OPERAND");
	case STATUS_FLOAT_DIVIDE_BY_ZERO:
		return TEXT("STATUS_FLOAT_DIVIDE_BY_ZERO");
	case STATUS_FLOAT_INEXACT_RESULT:
		return TEXT("STATUS_FLOAT_INEXACT_RESULT");
	case STATUS_FLOAT_INVALID_OPERATION:
		return TEXT("STATUS_FLOAT_INVALID_OPERATION");
	case STATUS_FLOAT_MULTIPLE_FAULTS:
		return TEXT("STATUS_FLOAT_MULTIPLE_FAULTS");
	case STATUS_FLOAT_MULTIPLE_TRAPS:
		return TEXT("STATUS_FLOAT_MULTIPLE_TRAPS");
	case STATUS_FLOAT_OVERFLOW:
		return TEXT("STATUS_FLOAT_OVERFLOW");
	case STATUS_FLOAT_STACK_CHECK:
		return TEXT("STATUS_FLOAT_STACK_CHECK");
	case STATUS_FLOAT_UNDERFLOW:
		return TEXT("STATUS_FLOAT_UNDERFLOW");
	case STATUS_GUARD_PAGE_VIOLATION:
		return TEXT("STATUS_GUARD_PAGE_VIOLATION");
	case STATUS_ILLEGAL_INSTRUCTION:
		return TEXT("STATUS_ILLEGAL_INSTRUCTION");
	case STATUS_IN_PAGE_ERROR:
		return TEXT("STATUS_IN_PAGE_ERROR");
	case STATUS_INVALID_DISPOSITION:
		return TEXT("STATUS_INVALID_DISPOSITION");
	case STATUS_INTEGER_DIVIDE_BY_ZERO:
		return TEXT("STATUS_INTEGER_DIVIDE_BY_ZERO");
	case STATUS_INTEGER_OVERFLOW:
		return TEXT("STATUS_INTEGER_OVERFLOW");
	case STATUS_NONCONTINUABLE_EXCEPTION:
		return TEXT("STATUS_NONCONTINUABLE_EXCEPTION");
	case STATUS_PRIVILEGED_INSTRUCTION:
		return TEXT("STATUS_PRIVILEGED_INSTRUCTION");
	case STATUS_REG_NAT_CONSUMPTION:
		return TEXT("STATUS_REG_NAT_CONSUMPTION");
	case STATUS_SINGLE_STEP:
		return TEXT("STATUS_SINGLE_STEP");
	case STATUS_STACK_OVERFLOW:
		return TEXT("STATUS_STACK_OVERFLOW");
	case CUSTOM_STATUS_SUCCESS:
		return TEXT("CUSTOM_STATUS_SUCCESS");
	case CUSTOM_STATUS_TERMINATED_BY_CHECKER:
		return TEXT("CUSTOM_STATUS_TERMINATED_BY_CHECKER");
	default:
		return TEXT("UNKNOWN_STATUS");
	}
}

// Given message identifier, return general verdict and, if possible, comment
VERDICT_CODE GetVerdictCode(
	DWORD dwNumberOfBytesAsMessageIdentifier,
	DWORD uExitCode)
{
	switch(dwNumberOfBytesAsMessageIdentifier)
	{
	case JOB_OBJECT_MSG_EXIT_PROCESS:
		return uExitCode == 0 ? VERDICT_SUCCESS : VERDICT_RUNTIME_ERROR;
	case JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS:
		return VERDICT_RUNTIME_ERROR;
	case JOB_OBJECT_MSG_END_OF_JOB_TIME:
	case JOB_OBJECT_MSG_END_OF_PROCESS_TIME:
	case CUSTOM_JOB_OBJECT_MSG_END_OF_JOB_TIME:
	case CUSTOM_JOB_OBJECT_MSG_END_OF_PROCESS_TIME:
		return VERDICT_TIME_LIMIT_EXCEEDED;
	case JOB_OBJECT_MSG_JOB_MEMORY_LIMIT:
	case JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT:
	case CUSTOM_JOB_OBJECT_MSG_JOB_MEMORY_LIMIT:
	case CUSTOM_JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT:
		return VERDICT_MEMORY_LIMIT_EXCEEDED;
	default:
		return VERDICT_UNKNOWN;
	}
}

// Given message identifier, return general verdict and, if possible, comment
void DispatchQueuedCompletionStatusToVerdict(
	DWORD dwNumberOfBytesAsMessageIdentifier,
	HANDLE hProcess,
	PVERDICT pVerdict)
{
	UINT uExitCode;

	TrueOrExit(GetExitCodeProcess(hProcess, &uExitCode));
	pVerdict->verdictCode = GetVerdictCode(dwNumberOfBytesAsMessageIdentifier, uExitCode);
	pVerdict->exitCodeMessage = GetExitCodeMessage(uExitCode);
}

#endif // DISPATCHER_H