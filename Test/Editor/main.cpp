// Editor main entry point. It's specific to Win32 platform !
#include "Editor.hpp"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE pInstance, LPSTR str, int showCmd)
{
	Editor editor;
	editor.Run();

	return 0;
}