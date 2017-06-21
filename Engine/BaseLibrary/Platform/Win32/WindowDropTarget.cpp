#include "WindowDropTarget.hpp"
#include "Window.hpp"

#include <Windows.h>
#include <vector>
#include <string>

WindowDropTarget::WindowDropTarget()
:m_cRef(1)
{
}

WindowDropTarget::~WindowDropTarget()
{
}

void WindowDropTarget::Init(Window* wnd)
{
	this->wnd = wnd;
}

STDMETHODIMP WindowDropTarget::QueryInterface(REFIID riid, void **ppv)
{
	if (riid == IID_IUnknown || riid == IID_IDropTarget) {
		*ppv = static_cast<IUnknown*>(this);
		AddRef();
		return S_OK;
	}
	*ppv = NULL;
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) WindowDropTarget::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) WindowDropTarget::Release()
{
	LONG cRef = InterlockedDecrement(&m_cRef);
	if (cRef == 0) delete this;
	return cRef;
}

STDMETHODIMP WindowDropTarget::DragEnter(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	dragData.Reset();
	
	FORMATETC textFormat = { CF_UNICODETEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };;
	FORMATETC fileFormat = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

	STGMEDIUM medium;

	// Drop text data
	if (pdto->GetData(&textFormat, &medium) == S_OK)
	{
		// We need to lock the HGLOBAL handle because we can't be sure if this is GMEM_FIXED (i.e. normal heap) data or not
		wchar_t* text = (wchar_t*)GlobalLock(medium.hGlobal);

		dragData.text = text;

		wnd->onDragEntered(dragData);

		GlobalUnlock(medium.hGlobal);
		ReleaseStgMedium(&medium);
	}
	else if (pdto->GetData(&fileFormat, &medium) == S_OK)
	{
		int fileCount = DragQueryFile((HDROP)medium.hGlobal, 0xFFFFFFFF, nullptr, 0);

		std::vector<path> filePaths;
		for (int i = 0; i < fileCount; ++i)
		{
			int FileNameLength = DragQueryFile((HDROP)medium.hGlobal, i, nullptr, 0);
			wchar_t* fileName = (wchar_t*)alloca(sizeof(wchar_t) * (FileNameLength + 1));
			DragQueryFile((HDROP)medium.hGlobal, i, fileName, FileNameLength + 1);
			filePaths.push_back(fileName);
		}

		dragData.filesPaths = std::move(filePaths);

		wnd->onDragEntered(dragData);

		ReleaseStgMedium(&medium);
	}

	*pdwEffect &= DROPEFFECT_COPY;
	return S_OK;
}

STDMETHODIMP WindowDropTarget::DragOver(DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	wnd->onDragHovering(dragData);

	*pdwEffect &= DROPEFFECT_COPY;
	return S_OK;
}

STDMETHODIMP WindowDropTarget::DragLeave()
{
	wnd->onDragLeaved(dragData);

	return S_OK;
}

STDMETHODIMP WindowDropTarget::Drop(IDataObject *pdto, DWORD grfKeyState, POINTL ptl, DWORD *pdwEffect)
{
	wnd->onDropped(dragData);

	*pdwEffect &= DROPEFFECT_COPY;
	return S_OK;
}