// Window implementation on [Windows OS]
#pragma once

#define NOMINMAX
#include <windows.h>

class Window;

// Class for drag & drop, it's member functions are called as event callbacks by OS when OS drag & drop occur
class WindowDropTarget : public IDropTarget
{
public:
	WindowDropTarget();
	~WindowDropTarget();

	void Init(Window* wnd);
	
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
	
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

	STDMETHODIMP DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);
	STDMETHODIMP DragLeave();
	STDMETHODIMP Drop(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect);

private:
	long m_cRef;
	Window* wnd;
};