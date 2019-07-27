#pragma once


#include "ComponentFactory.hpp"


namespace inl::game {


template <class ComponentT, const char* ClassName, class FactoryT = ComponentClassFactory<ComponentT>>
class AutoRegisterComponent {
private:
	static int Register() {
		ComponentFactory_Singleton::GetInstance().Register<ComponentT, FactoryT>(ClassName);
		return 0;
	}
	inline static int ignored = Register();
};


} // namespace inl::game