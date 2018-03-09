#pragma once

#include <memory>

namespace inl {

template <class T>
class Singleton : public T {
public:
	static Singleton& GetInstance() {
		static std::unique_ptr<Singleton> instance(new Singleton());
		return *instance.get();
	}
};

}