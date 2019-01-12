#pragma once

#include <memory>

namespace inl::gui {


template <class T>
class SharedPtrLess {
public:
	bool operator()(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs) const {
		return lhs < rhs;
	}
	bool operator()(const std::shared_ptr<T>& lhs, const T* rhs) const {
		return lhs.get() < rhs;
	}
	bool operator()(const T* lhs, const std::shared_ptr<T>& rhs) const {
		return lhs < rhs.get();
	}
	using is_transparent = void*;
};


} // namespace inl::gui