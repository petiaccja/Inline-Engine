#pragma once

#include <memory>


namespace inl::gui {


template <class T>
std::shared_ptr<T> MakeBlankShared(T& obj) {
	return std::shared_ptr<T>(&obj, [](auto){});
}


} // namespace inl::gui