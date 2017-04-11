
#include "../Sys.hpp"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <xlocbuf>
#include <codecvt>

#include <iostream>
#include <tchar.h>

Sys::DLLHandle Sys::LoadDLL(const wchar_t* path) 
{
	auto value = LoadLibraryW(path);
	return value;
}

bool Sys::UnLoadDLL(DLLHandle h) 
{
	return FreeLibrary((HMODULE)h) ? true : false;
}

void Sys::ShowMsgBox(const std::wstring& msg)
{
	MessageBoxW(0, msg.c_str(), L"", MB_OK);

}

void Sys::SetCursorPos(const Vector2i& pos)
{
	::SetCursorPos(pos.x(), pos.y());
}

void Sys::SetCursorVisible(bool b)
{
	ShowCursor(b ? SW_SHOW : SW_HIDE);
}

void* Sys::GetDLLProcAddress(DLLHandle h, const std::string& procName) 
{
	return (void*)GetProcAddress((HMODULE)h, procName.c_str());
}

// TODO REMOVE
std::wstring Sys::GetExeDirW()
{
	std::string exeDir = GetExeDir();
	return std::wstring(exeDir.begin(), exeDir.end());
}

std::string	Sys::GetExeDir()
{
	// TODO make define or constant somewhere (128)
	char path[256];
	memset(path, 0, sizeof(path));
	GetModuleFileNameA(0, path, 256);

	size_t i = 255;
	while (i > 0 && path[i] != '/' && path[i] != '\\')
		i--;

	path[i + 1] = 0;

	size_t idx = 0;
	while (path[idx] != '\0') 
	{
		if (path[idx] == '\\')
			path[idx] = '/';

		idx++;
	}

	return path;
}

Vector2i Sys::GetCursorPos()
{
	POINT p; ::GetCursorPos(&p);
	return Vector2i(p.x, p.y);
}

Vector2u Sys::GetScreenSize()
{
	return Vector2u((unsigned)GetSystemMetrics(SM_CXSCREEN), (unsigned)GetSystemMetrics(SM_CYSCREEN));
}