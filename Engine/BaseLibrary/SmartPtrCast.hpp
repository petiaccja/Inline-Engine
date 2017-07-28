#pragma once


#include <memory>


namespace inl {



template <class DestT, class SrcT, class SrcDeleter>
std::unique_ptr<DestT, SrcDeleter> static_cast_smart(std::unique_ptr<SrcT, SrcDeleter>&& src) {
	DestT* dst = static_cast<DestT*>(src.get());
	SrcDeleter deleter = std::move(src.get_deleter());
	src.release();

	std::unique_ptr<DestT, SrcDeleter> ret{ dst, std::move(deleter) };
	return ret;
}


template <class DestT, class SrcT, class SrcDeleter>
std::unique_ptr<DestT, SrcDeleter> dynamic_cast_smart(std::unique_ptr<SrcT, SrcDeleter>&& src) {
	DestT* dst = dynamic_cast<DestT*>(src.get());
	if (dst == nullptr) {
		return {};
	}

	SrcDeleter deleter = std::move(src.get_deleter());
	src.release();

	std::unique_ptr<DestT, SrcDeleter> ret{ dst, std::move(deleter) };
	return ret;
}



} // namespace inl