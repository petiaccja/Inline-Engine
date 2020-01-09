#pragma once


#include "Archive.hpp"


namespace inl::game {



class LevelInputArchive : public ModuleArchive, public cereal::InputArchive<LevelInputArchive> {
public:
	template <class... Args>
	LevelInputArchive(Args&&... args) : cereal::InputArchive<LevelInputArchive>(this), base(std::forward<Args>(args)...) {}

	game::InputArchive& Base() {
		return base;
	}

private:
	game::InputArchive base;
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


class LevelOutputArchive : public ModuleArchive, public cereal::OutputArchive<LevelOutputArchive> {
public:
	template <class... Args>
	LevelOutputArchive(Args&&... args) : cereal::OutputArchive<LevelOutputArchive>(this), base(std::forward<Args>(args)...) {}

	game::OutputArchive& Base() {
		return base;
	}

private:
	game::OutputArchive base;
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