#pragma once

#include <type_traits>


namespace inl::game {

class SystemBase;


class HookBase {
public:
	virtual void Run(SystemBase& system) = 0;
};


template <class SystemInterface>
class Hook : public HookBase {
	static_assert(!std::is_same_v<SystemInterface, SystemBase>);

public:
	virtual void Run(SystemInterface& system) = 0;

private:
	void Run(SystemBase& system) override;
};


template <class SystemInterface>
void Hook<SystemInterface>::Run(SystemBase& system) {
	SystemInterface* systemInterface = dynamic_cast<SystemInterface*>(&system);
	if (systemInterface != nullptr) {
		Run(*systemInterface);
	}
}

} // namespace inl::game
