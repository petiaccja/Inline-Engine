#pragma once



#if defined(WIN32) && defined(_MSC_VER)

#include <Windows.h>

namespace inl {

namespace impl {
// Usage: SetThreadName ("CurrentThread");
//        SetThreadName ("OtherThread", 4567);
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // must be 0x1000
	LPCSTR szName; // pointer to name (in user addr space)
	DWORD dwThreadID; // thread ID (-1=caller thread)
	DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

inline void SetThreadName(LPCSTR szThreadName, CONST DWORD dwThreadID = -1)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;
	if (IsDebuggerPresent()) {
		__try
		{
			RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
		}
		__except (EXCEPTION_CONTINUE_EXECUTION) {}
	}
}
}

inline void SetCurrentThreadName(const char* name) {
	impl::SetThreadName(name, GetCurrentThreadId());
}
}

#else

inline void SetCurrentThreadName(const char* name) {
	// thread name can only be set on windows, with visual studio
}

#endif

