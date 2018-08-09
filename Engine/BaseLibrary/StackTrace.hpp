#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <mutex>


template <template <class> class Allocator = std::allocator>
struct StackFrameT {
	int frame;
	void* instructionAddress;
	void* stackAddress;
	void* frameAddress;
	std::basic_string<char, std::char_traits<char>, Allocator<char>> symbol;
	std::basic_string<char, std::char_traits<char>, Allocator<char>> sourceFile;
	int sourceLine;
};


using StackFrame = StackFrameT<>;


template <template <class> class Allocator = std::allocator>
std::ostream& operator<<(std::ostream& os, const StackFrameT<Allocator>& frame) {
	os << frame.instructionAddress << " " << frame.symbol << " (" << frame.sourceFile << ":" << frame.sourceLine << ")";
	return os;
}


#if defined(_WIN32) && defined(_M_AMD64)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Dbghelp.h>

template <template <class> class Allocator = std::allocator>
std::vector<StackFrameT<Allocator>, Allocator<StackFrameT<Allocator>>> GetStackTrace() {
	// Initialize sym stuff
	[[maybe_unused]]
	static bool isInitalized = [] {
		SymInitialize(GetCurrentProcess(), NULL, TRUE);
		return true;
	}();


	// Sym stuff is single-threaded, lock it
	static std::mutex mtx;
	std::lock_guard<std::mutex> lkg(mtx);


	// Helper to have moar characters
	struct IMAGEHLP_SYMBOL64_EXTRA {
		IMAGEHLP_SYMBOL64 data;
		char nameExtra[1024] = { 0 };
	};

	
	// Get process and thread
	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();

	// Capture context
	CONTEXT context;
	STACKFRAME64 stack;
	RtlCaptureContext(&context);
	memset(&stack, 0, sizeof(STACKFRAME64));

	stack.AddrPC.Offset = context.Rip;
	stack.AddrPC.Mode = AddrModeFlat;
	stack.AddrStack.Offset = context.Rsp;
	stack.AddrStack.Mode = AddrModeFlat;
	stack.AddrFrame.Offset = context.Rbp;
	stack.AddrFrame.Mode = AddrModeFlat;

	// Iterate over stack frames
	std::vector<StackFrameT<Allocator>, Allocator<StackFrameT<Allocator>>> frames;
	for (int frame = 0; ; frame++) {
		BOOL result;
		result = StackWalk64(IMAGE_FILE_MACHINE_AMD64,
							 process,
							 thread,
							 &stack,
							 &context,
							 NULL,
							 SymFunctionTableAccess64,
							 SymGetModuleBase64,
							 NULL);

		// Get symbol name
		IMAGEHLP_SYMBOL64_EXTRA symbol;
		char undecoratedName[256];
		DWORD64 displacement64 = 0;

		symbol.data.SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
		symbol.data.MaxNameLength = 255;

		SymGetSymFromAddr64(process, (ULONG64)stack.AddrPC.Offset, &displacement64, &symbol.data);
		UnDecorateSymbolName(symbol.data.Name, (PSTR)undecoratedName, 256, UNDNAME_COMPLETE);

		// Get file-line information
		DWORD displacement = 0;
		IMAGEHLP_LINE64 lineInfo;
		lineInfo.Address = stack.AddrPC.Offset;
		lineInfo.Key = nullptr;
		lineInfo.SizeOfStruct = sizeof(lineInfo);
		lineInfo.FileName = nullptr;
		lineInfo.LineNumber = -1;
		SymSetOptions(SYMOPT_LOAD_LINES);
		SymGetLineFromAddr64(process, stack.AddrPC.Offset, &displacement, &lineInfo);


		StackFrameT<Allocator> currentFrame;
		currentFrame.frame = frame;
		currentFrame.frameAddress = (void*)stack.AddrFrame.Offset;
		currentFrame.instructionAddress = (void*)stack.AddrPC.Offset;
		currentFrame.stackAddress = (void*)stack.AddrStack.Offset;
		currentFrame.symbol = undecoratedName;
		currentFrame.sourceFile = lineInfo.FileName ? lineInfo.FileName : "<no file info>";
		currentFrame.sourceLine = lineInfo.LineNumber != (DWORD)-1 ? lineInfo.LineNumber : 0;
		frames.push_back(currentFrame);

		if (!result)
		{
			break;
		}
	}

	if (frames.size() > 0) {
		frames = decltype(frames)(frames.begin() + 1, frames.end());
	}

	return frames;
}

#else

template <template <class> class Allocator = std::allocator>
std::vector<StackFrameT, Allocator> GetStackTrace() {
	StackFrameT<Allocator> currentFrame;
	currentFrame.frame = 0;
	currentFrame.frameAddress = (void*)0;
	currentFrame.instructionAddress = (void*)0;
	currentFrame.stackAddress = (void*)0;
	currentFrame.symbol = "no stack trace for this platform";
	currentFrame.sourceFile = "<no file info>";
	currentFrame.sourceLine = 0;

	return { currentFrame };
}

#endif