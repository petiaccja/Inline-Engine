// Window implementation on [Windows OS]
#pragma once

#include "../WindowCommon.hpp"

#define NOMINMAX
#include <windows.h>
#include <queue>
#include <functional>

// TODO later we will need this for fully functional Drag&Drop
//class WindowDropTarget : public IDropTarget
//{
//public:
//	WindowDropTarget() : m_cRef(1) { }
//	~WindowDropTarget() { }
//
//	// *** IUnknown ***
//	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
//	{
//		if (riid == IID_IUnknown || riid == IID_IDropTarget) {
//			*ppv = static_cast<IUnknown*>(this);
//			AddRef();
//			return S_OK;
//		}
//		*ppv = NULL;
//		return E_NOINTERFACE;
//	}
//
//	STDMETHODIMP_(ULONG) AddRef()
//	{
//		return InterlockedIncrement(&m_cRef);
//	}
//
//	STDMETHODIMP_(ULONG) Release()
//	{
//		LONG cRef = InterlockedDecrement(&m_cRef);
//		if (cRef == 0) delete this;
//		return cRef;
//	}
//	//Next come the methods of IDropTarget, none of which are particularly interesting.We just say that we are going to copy the data.
//
//		// *** IDropTarget ***
//	STDMETHODIMP DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
//	{
//		*pdwEffect &= DROPEFFECT_COPY;
//		return S_OK;
//	}
//
//	STDMETHODIMP DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
//	{
//		*pdwEffect &= DROPEFFECT_COPY;
//		return S_OK;
//	}
//
//	STDMETHODIMP DragLeave()
//	{
//		return S_OK;
//	}
//
//	STDMETHODIMP Drop(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
//	{
//		
//		*pdwEffect &= DROPEFFECT_COPY;
//		return S_OK;
//	}
//
//private:
//	long m_cRef;
//};

class Window
{
public:
	Window(const WindowDesc& d);

	bool PopEvent(WindowEvent& evt_out);

	void Close();
	void MinimizeSize();
	void MaximizeSize();
	void RestoreSize();


	void SetRect(const Vector2i& pos, const Vector2u& size);
	void SetPos(const Vector2i& pos);
	void SetSize(const Vector2u& size);

	void SetTitle(const std::string& text);
	void SetIcon(const std::wstring& filePath);

	// Getters
	bool IsOpen() const;
	bool IsFocused() const;
	bool IsMaximizedSize() const;
	bool IsMinimizedSize() const;

	WindowHandle GetHandle() const;

	uint32_t GetClientWidth() const;
	uint32_t GetClientHeight() const;
	Vector2u GetClientSize() const;
	Vector2f GetClientCursorPos() const;

	unsigned GetNumClientPixels() const;
	float GetClientAspectRatio() const;

	Vector2i GetCenterPos() const;

	const std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>& GetUserWndProc() const { return userWndProc; }

public:
	// HEKK
	Delegate<void()> hekkOnPaint;

	Delegate<void(WindowEvent&)>	onMousePressed;
	Delegate<void(WindowEvent&)>	onMouseReleased;
	Delegate<void(WindowEvent&)>	onMouseMoved;
	Delegate<void(Vector2u)>		onClientSizeChanged;
	Delegate<void(std::vector<std::wstring>&)> onDrop;

protected:
	friend LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	eKey ConvertFromWindowsKey(WPARAM key);

protected:
	HWND handle;
	bool bClosed;
	std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)> userWndProc;
	//WindowDropTarget dropTarget; // TODO later we will need this for fully functional Drag&Drop
};