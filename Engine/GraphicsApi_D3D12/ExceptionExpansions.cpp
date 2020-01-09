#include "ExceptionExpansions.hpp"

#include "d3dx12.h"

#include "../GraphicsApi_LL/Exception.hpp"

#include <comdef.h>
#include <string>

namespace inl::gxapi_dx12 {


void ThrowIfFailed(HRESULT code, const std::string& additionalInfo) {
	if (!FAILED(code)) {
		return;
	}

	std::string msg;
	{
		_com_error err(code);
		//dirty hack to convert to char string.
		const TCHAR* curr = err.ErrorMessage();
		while (*curr != 0) {
			msg.push_back(char(*(curr++)));
		}
		//msg += " Code: " + std::to_string(code);
	}

	if (additionalInfo.length() > 0) {
		msg += "\n\tAdditional info: " + additionalInfo;
	}

	switch (code) {
			//these do not exsit. (what the ??)
			//case D3D12_ERROR_FILE_NOT_FOUND:
			//	break;
			//case D3D12_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
			//	break;
			//case D3D12_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS:
			//	break;
		case DXGI_ERROR_INVALID_CALL:
			throw InvalidCallException(std::move(msg));
			break;
		case DXGI_ERROR_WAS_STILL_DRAWING:
			throw InvalidStateException(msg + "\n\tDetails: The previous blit operation that is transferring information to or from this surface is incomplete.");
			break;
		case E_FAIL:
			throw Exception(msg + "\n\tDetails: Attempted to create a device with the debug layer enabled and the layer is not installed.");
			break;
		case E_INVALIDARG:
			throw InvalidArgumentException(std::move(msg));
			break;
		case E_OUTOFMEMORY:
			throw OutOfMemoryException(std::move(msg));
			break;
		case E_NOTIMPL:
			throw NotImplementedException(std::move(msg));
			break;
		case S_FALSE:
		default:
			throw Exception(std::move(msg));
			break;
	}
}


} // namespace inl::gxapi_dx12
