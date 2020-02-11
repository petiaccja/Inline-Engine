#pragma once

#include <type_traits>


namespace inl::game {

class SystemBase;


class HookBase {
public:
	virtual void BeginFrame() = 0;
	virtual void PreRun(SystemBase& system) = 0;
	virtual void PostRun(SystemBase& system) = 0;
	virtual void EndFrame() = 0;
};


template <class SystemInterface>
class Hook : public HookBase {
	static_assert(!std::is_same_v<SystemInterface, SystemBase>);

public:
	void BeginFrame() override {}
	virtual void PreRun(SystemInterface& system) {}
	virtual void PostRun(SystemInterface& system) {}
	void EndFrame() override {}

private:
	void PreRun(SystemBase& system) override final;
	void PostRun(SystemBase& system) override final;
};


template <class SystemInterface>
void Hook<SystemInterface>::PreRun(SystemBase& system) {
	SystemInterface* systemInterface = dynamic_cast<SystemInterface*>(&system);
	if (systemInterface != nullptr) {
		PreRun(*systemInterface);
	}
}

template <class SystemInterface>
void Hook<SystemInterface>::PostRun(SystemBase& system) {
	SystemInterface* systemInterface = dynamic_cast<SystemInterface*>(&system);
	if (systemInterface != nullptr) {
		PostRun(*systemInterface);
	}
}


} // namespace inl::game
