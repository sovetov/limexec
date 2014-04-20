#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "common_compile_defines.h"

#include <Windows.h>
#include "debug.h"
#include "custom_job_object_msgs.h"

typedef int VERDICT_CODE;
#define VERDICT_SUCCESS 0
#define VERDICT_RUNTIME_ERROR 1
#define VERDICT_TIME_LIMIT_EXCEEDED 2
#define VERDICT_MEMORY_LIMIT_EXCEEDED 3
#define VERDICT_IDLENESS_LIMIT_EXCEEDED 6
#define VERDICT_UNKNOWN_MESSAGE 7
#define VERDICT_ACCESS_DENIED 8
#define VERDICT_THAT_SHOULD_NOT_BE_REACHED 9
#define VERDICT_UNKNOWN_EXIT_CODE 10

typedef struct _VERDICT {
	VERDICT_CODE verdictCode;
	LPCTSTR exitCodeMessage, messageMessage;
	DWORD exitCode;
} VERDICT, *PVERDICT;

#define NTSTATUS_STATUS_SUCCESS 0
#define NTSTATUS_STATUS_ACCESS_DENIED 0xC0000022
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
	case CUSTOM_STATUS_TERMINATED_BY_CHECKER:
		return TEXT("CUSTOM_STATUS_TERMINATED_BY_CHECKER");
	case STATUS_INVALID_CRUNTIME_PARAMETER:
		return TEXT("STATUS_INVALID_CRUNTIME_PARAMETER");

	case NTSTATUS_STATUS_SUCCESS:
		return TEXT("CUSTOM_STATUS_SUCCESS");
	case NTSTATUS_STATUS_ACCESS_DENIED:
		return TEXT("STATUS_ACCESS_DENIED");

	default:
		return TEXT("UNKNOWN_STATUS");
	}
}

LPCTSTR GetMessageMessage(DWORD dwNumberOfbytesAsMessageIdentifier)
{
	switch(dwNumberOfbytesAsMessageIdentifier)
	{
	case JOB_OBJECT_MSG_END_OF_JOB_TIME:
		return TEXT("JOB_OBJECT_MSG_END_OF_JOB_TIME");
	case JOB_OBJECT_MSG_END_OF_PROCESS_TIME:
		return TEXT("JOB_OBJECT_MSG_END_OF_PROCESS_TIME");
	case JOB_OBJECT_MSG_ACTIVE_PROCESS_LIMIT:
		return TEXT("JOB_OBJECT_MSG_ACTIVE_PROCESS_LIMIT");
	case JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO:
		return TEXT("JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO");
	case JOB_OBJECT_MSG_NEW_PROCESS:
		return TEXT("JOB_OBJECT_MSG_NEW_PROCESS");
	case JOB_OBJECT_MSG_EXIT_PROCESS:
		return TEXT("JOB_OBJECT_MSG_EXIT_PROCESS");
	case JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS:
		return TEXT("JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS");
	case JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT:
		return TEXT("JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT");
	case JOB_OBJECT_MSG_JOB_MEMORY_LIMIT:
		return TEXT("JOB_OBJECT_MSG_JOB_MEMORY_LIMIT");

	case CUSTOM_JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT:
		return TEXT("CUSTOM_JOB_OBJECT_MSG_PROCESS_MEMORY_LIMIT");
	case CUSTOM_JOB_OBJECT_MSG_JOB_MEMORY_LIMIT:
		return TEXT("CUSTOM_JOB_OBJECT_MSG_JOB_MEMORY_LIMIT");
	case CUSTOM_JOB_OBJECT_MSG_END_OF_JOB_TIME:
		return TEXT("CUSTOM_JOB_OBJECT_MSG_END_OF_JOB_TIME");
	case CUSTOM_JOB_OBJECT_MSG_END_OF_PROCESS_TIME:
		return TEXT("CUSTOM_JOB_OBJECT_MSG_END_OF_PROCESS_TIME");
	case CUSTOM_JOB_OBJECT_MSG_END_OF_IDLENESS:
		return TEXT("CUSTOM_JOB_OBJECT_MSG_END_OF_IDLENESS");

	default:
		return TEXT("MSG_UNKNOWN");
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
	case JOB_OBJECT_MSG_NEW_PROCESS:
		return VERDICT_THAT_SHOULD_NOT_BE_REACHED;
	case JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO:
		switch(uExitCode)
		{
		case NTSTATUS_STATUS_SUCCESS:
			return VERDICT_SUCCESS;
		case NTSTATUS_STATUS_ACCESS_DENIED:
			return VERDICT_ACCESS_DENIED;
		default:
			return VERDICT_UNKNOWN_EXIT_CODE;
		}
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
	case CUSTOM_JOB_OBJECT_MSG_END_OF_IDLENESS:
		return VERDICT_IDLENESS_LIMIT_EXCEEDED;
	default:
		return VERDICT_UNKNOWN_MESSAGE;
	}
}

// Given message identifier, return general verdict and, if possible, comment
void DispatchQueuedCompletionStatusToVerdict(
	DWORD dwNumberOfBytesAsMessageIdentifier,
	HANDLE hProcess,
	PVERDICT pVerdict)
{
	DWORD uExitCode;

	_ftprintf(stderr, TEXT("Getting exit code...\n"));
	fflush(stderr);
	TrueOrExit(TEXT("Executor. Dispatcher. WaitForSingleObject"), WAIT_OBJECT_0 == WaitForSingleObject(hProcess, INFINITE));
	TrueOrExit(TEXT("Executor. Dispatcher. GetExitCodeProcess"), GetExitCodeProcess(hProcess, &uExitCode));
	pVerdict->verdictCode = GetVerdictCode(dwNumberOfBytesAsMessageIdentifier, uExitCode);
	pVerdict->exitCode = uExitCode;
	pVerdict->exitCodeMessage = GetExitCodeMessage(uExitCode);
	pVerdict->messageMessage = GetMessageMessage(dwNumberOfBytesAsMessageIdentifier);
}

#endif // DISPATCHER_H