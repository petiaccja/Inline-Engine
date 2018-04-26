#pragma once

#include <type_traits>
#include <memory>


namespace inl {



template <class DestT, class SrcT, class SrcDeleter, class = std::enable_if_t<std::is_nothrow_move_assignable_v<SrcDeleter>>>
std::unique_ptr<DestT, SrcDeleter> static_pointer_cast(std::unique_ptr<SrcT, SrcDeleter>&& src) noexcept {
	DestT* dst = static_cast<DestT*>(src.get());
	SrcDeleter deleter = std::move(src.get_deleter());
	src.release();

	std::unique_ptr<DestT, SrcDeleter> ret{ dst, std::move(deleter) };
	return ret;
}


template <class DestT, class SrcT, class SrcDeleter, class = std::enable_if_t<std::is_nothrow_move_constructible_v<SrcDeleter>>>
std::unique_ptr<DestT, SrcDeleter> dynamic_pointer_cast(std::unique_ptr<SrcT, SrcDeleter>&& src) noexcept {
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