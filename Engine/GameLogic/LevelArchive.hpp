#pragma once

#include "VariantArchive.hpp"

#include <BaseLibrary/Container/DynamicTuple.hpp>

#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>


namespace inl::game {


using InputArchive = VariantInputArchive<cereal::JSONInputArchive, cereal::PortableBinaryInputArchive>;
using OutputArchive = VariantOutputArchive<cereal::JSONOutputArchive, cereal::PortableBinaryOutputArchive>;



class LevelInputArchive : public cereal::InputArchive<LevelInputArchive> {
public:
	template <class... Args>
	LevelInputArchive(std::shared_ptr<const DynamicTuple> modules, Args&&... args) : cereal::InputArchive<LevelInputArchive>(this), base(std::forward<Args>(args)...), modules(modules) {}

	game::InputArchive& Base() { return base; }

	const DynamicTuple& Modules() const { return *modules; }

private:
	game::InputArchive base;
	std::shared_ptr<const DynamicTuple> modules;
};


template <class T>
void prologue(LevelInputArchive& ar, const T& arg) {
	if constexpr (!HandleArchive<T>::value) {
		prologue(ar.Base(), arg);
	}
}

template <class T>
void epilogue(LevelInputArchive& ar, const T& arg) {
	if constexpr (!HandleArchive<T>::value) {
		epilogue(ar.Base(), arg);
	}
}

template <class T>
std::enable_if_t<HandleArchive<T>::value, void> CEREAL_LOAD_FUNCTION_NAME(LevelInputArchive& ar, T& arg) {
	ar.Base()(arg);
}

template <class CharT, class Traits, class Alloc>
void CEREAL_LOAD_FUNCTION_NAME(LevelInputArchive& ar, std::basic_string<CharT, Traits, Alloc>& arg) {
	ar.Base()(arg);
}


class LevelOutputArchive : public cereal::OutputArchive<LevelOutputArchive> {
public:
	template <class... Args>
	LevelOutputArchive(std::shared_ptr<const DynamicTuple> modules, Args&&... args) : cereal::OutputArchive<LevelOutputArchive>(this), base(std::forward<Args>(args)...), modules(modules) {}

	game::OutputArchive& Base() { return base; }

	const DynamicTuple& Modules() const { return *modules; }

private:
	game::OutputArchive base;
	std::shared_ptr<const DynamicTuple> modules;
};


template <class T>
void prologue(LevelOutputArchive& ar, const T& arg) {
	if constexpr (!HandleArchive<T>::value) {
		prologue(ar.Base(), arg);
	}
}

template <class T>
void epilogue(LevelOutputArchive& ar, const T& arg) {
	if constexpr (!HandleArchive<T>::value) {
		epilogue(ar.Base(), arg);
	}
}

template <class T>
std::enable_if_t<HandleArchive<T>::value, void> CEREAL_SAVE_FUNCTION_NAME(LevelOutputArchive& ar, const T& arg) {
	ar.Base()(arg);
}

template <class CharT, class Traits, class Alloc>
void CEREAL_SAVE_FUNCTION_NAME(LevelOutputArchive& ar, const std::basic_string<CharT, Traits, Alloc>& arg) {
	ar.Base()(arg);
}


} // namespace inl::game