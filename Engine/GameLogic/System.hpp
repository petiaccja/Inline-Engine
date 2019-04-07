#pragma once

#include "ComponentRange.hpp"


namespace inl::game {



template <class... ComponentTypes>
class System {
public:

protected:
	virtual void Update(ComponentRange<ComponentTypes...> componentList) = 0;
};


} // namespace inl::game