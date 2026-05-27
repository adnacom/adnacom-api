#pragma once

#include <windows.h>
#include <evntrace.h>


enum DebugLevel : ULONG
{
	lvlError = TRACE_LEVEL_ERROR,
	lvlWarning = TRACE_LEVEL_WARNING,
	lvlTrace = TRACE_LEVEL_INFORMATION,
	lvlInfo = TRACE_LEVEL_VERBOSE,
};


#ifdef _DEBUG
#	define TRACE_DEBUG_OUTPUT	1
#endif // debug build

#define printLn(_fmt, ...)	wprintf_s(TEXT(_fmt) L"\r\n", ##__VA_ARGS__)

#ifdef TRACE_DEBUG_OUTPUT
	void traceImpl_(const wchar_t* fmt, ...);
#	define traceLn_(_fmt, ...)	traceImpl_(TEXT(_fmt) L" @" __FILE__ ":%d\r\n", ##__VA_ARGS__, __LINE__)
#else
#	define traceLn_(_fmt, ...)	do { } while (false)
#endif


#define traceBase_(_lvl, _fmt, ...)	traceLn_(_fmt, ##__VA_ARGS__)

#define traceErr(_fmt, ...) traceBase_(lvlError, "[!] " _fmt, ##__VA_ARGS__)
#define traceWarn(_fmt, ...) traceBase_(lvlWarning, "/!\\ " _fmt, ##__VA_ARGS__)
#define trace(_fmt, ...)	traceBase_(lvlTrace, _fmt, ##__VA_ARGS__)
#define traceInfo(fmt, ...) traceBase_(lvlInfo, _fmt, ##__VA_ARGS__)
