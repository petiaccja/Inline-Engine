#pragma once

#include "VariantArchive.hpp"

#include "BaseLibrary/DynamicTuple.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

namespace inl::game {


class ModuleArchive {
public:
	template <class T>
	const T* GetModule() const {
		return m_modules.Get<const T*>();
	}

	template <class T>
	void RegisterModule(const T* module) {
		m_modules.Insert(module);
	}

private:
	DynamicTuple m_modules;
};


using InputArchive = VariantInputArchive<cereal::JSONInputArchive, cereal::PortableBinaryInputArchive>;
using OutputArchive = VariantOutputArchive<cereal::JSONOutputArchive, cereal::PortableBinaryOutputArchive>;

} // namespace inl::game
