#pragma once

#include <memory>

namespace exc {

template <class T>
class Singleton : public T {
public:
	static Singleton* GetInstance() {
		static unique_ptr<Singleton> instance(new Singleton());
		return instance.get();
	}
};

}