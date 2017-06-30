// Editor main entry point. It's specific to Win32 platform !
#include "Editor.hpp"
#include <assert.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE pInstance, LPSTR str, int showCmd)
{
	Editor editor;
	editor.Start();

	return 0;
}