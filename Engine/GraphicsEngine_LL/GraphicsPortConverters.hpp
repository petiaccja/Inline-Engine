#pragma once

// This file contains all the port converters related to graphics nodes.
// This is required for loading and exporting JSON pipeline descriptions.


#include <BaseLibrary/Graph/Port.hpp>

#include <unordered_set>
#include <vector>

// Graphics API & Engine includes
#include <GraphicsApi_LL/Common.hpp>
#include "NodeContext.hpp"
#include "Scene.hpp"



namespace inl {


// Utility class for enum <-> string conversion.
template <class EnumT, std::vector<std::pair<EnumT, std::string>>(*Generator)()>
class EnumConverter {
public:
	static std::string ToString(const EnumT& arg);
	static EnumT FromString(const std::string& arg);
protected:
	using Record = std::pair<EnumT, std::string>;

	struct FmtHash {
		size_t operator()(const Record* obj) const { return std::hash<EnumT>()(obj->first); }
		size_t operator()(const Record* lhs, const Record* rhs) const { return lhs->first == rhs->first; }
	};
	struct StrHash {
		size_t operator()(const Record* obj) const { return std::hash<std::string>()(obj->second); }
		size_t operator()(const Record* lhs, const Record* rhs) const { return lhs->second == rhs->second; }
	};

	static void GetMaps(const std::unordered_set<const Record*, FmtHash, FmtHash>** byFmt,
		const std::unordered_set<const Record*, StrHash, StrHash>** byStr);
};


template<class EnumT, std::vector<std::pair<EnumT, std::string>>(*Generator)()>
inline std::string EnumConverter<EnumT, Generator>::ToString(const EnumT & arg) {
	const std::unordered_set<const Record*, FmtHash, FmtHash>* byFmt;
	const std::unordered_set<const Record*, StrHash, StrHash>* byStr;

	GetMaps(&byFmt, &byStr);

	Record key{ arg, {} };
	auto it = byFmt->find(&key);
	if (it != byFmt->end()) {
		return (*it)->second;
	}
	else {
		throw OutOfRangeException("Could not translate format. Update table.");
	}
}

template<class EnumT, std::vector<std::pair<EnumT, std::string>>(*Generator)()>
inline EnumT EnumConverter<EnumT, Generator>::FromString(const std::string & arg) {
	const std::unordered_set<const Record*, FmtHash, FmtHash>* byFmt;
	const std::unordered_set<const Record*, StrHash, StrHash>* byStr;

	GetMaps(&byFmt, &byStr);

	Record key{ {}, arg };
	auto it = byStr->find(&key);
	if (it != byStr->end()) {
		return (*it)->first;
	}
	else {
		throw OutOfRangeException("Could not translate string. Update table or user error.");
	}
}

template<class EnumT, std::vector<std::pair<EnumT, std::string>>(*Generator)()>
inline void EnumConverter<EnumT, Generator>::GetMaps(
	const std::unordered_set<const Record*, FmtHash, FmtHash>** byFmt,
	const std::unordered_set<const Record*, StrHash, StrHash>** byStr) 
{
	static std::vector<Record> records = Generator();

	static std::unordered_set<const Record*, FmtHash, FmtHash> byFmtData = [&] {
		std::unordered_set<const Record*, FmtHash, FmtHash> tmp;
		for (const auto& rec : records) {
			tmp.insert(&rec);
		}
		return tmp;
	}();
	static std::unordered_set<const Record*, StrHash, StrHash> byStrData = [&] {
		std::unordered_set<const Record*, StrHash, StrHash> tmp;
		for (const auto& rec : records) {
			tmp.insert(&rec);
		}
		return tmp;
	}();

	*byFmt = &byFmtData;
	*byStr = &byStrData;
}

namespace impl {
	// Generators for common enums
	template <class EnumT>
	std::vector<std::pair<EnumT, std::string>> ParseTableGenerator() {
		static_assert(false, "Enum parse table generators have to be specialized and coded by hand.");
	}

	template <>
	std::vector<std::pair<gxapi::eFormat, std::string>> ParseTableGenerator();

	template <>
	std::vector<std::pair<gxapi::eBlendOperation, std::string>> ParseTableGenerator();

	template <>
	std::vector<std::pair<gxapi::eBlendOperand, std::string>> ParseTableGenerator();

	template <>
	std::vector<std::pair<gxapi::eBlendLogicOperation, std::string>> ParseTableGenerator();
	
}


// eFormat
template <>
class PortConverter<gxapi::eFormat> : public PortConverterCollection<gxapi::eFormat> {
public:
	PortConverter() :
		PortConverterCollection<gxapi::eFormat>(&FromString) {}

	std::string ToString(const gxapi::eFormat& fmt) const override {
		return EnumConverter<gxapi::eFormat, &impl::ParseTableGenerator<gxapi::eFormat>>::ToString(fmt);
	}

	static gxapi::eFormat FromString(const std::string& str) {
		return EnumConverter<gxapi::eFormat, &impl::ParseTableGenerator<gxapi::eFormat>>::FromString(str);
	}
};



// RenderTargetBlendState
template <>
class PortConverter<gxapi::RenderTargetBlendState> : public PortConverterCollection<gxapi::RenderTargetBlendState> {
public:
	PortConverter() :
		PortConverterCollection<gxapi::RenderTargetBlendState>(&FromString) {}

	std::string ToString(const gxapi::RenderTargetBlendState& fmt) const override;
protected:
	static gxapi::RenderTargetBlendState FromString(const std::string&);
};



// TextureUsage
template <>
class PortConverter<gxeng::TextureUsage> : public PortConverterCollection<gxeng::TextureUsage> {
public:
	PortConverter() :
		PortConverterCollection<gxeng::TextureUsage>(&FromString) {}

	std::string ToString(const gxeng::TextureUsage& fmt) const override;
protected:
	static gxeng::TextureUsage FromString(const std::string&);
};


// Scene -> EntityCollection
template <class EntityType>
class PortConverter<const gxeng::EntityCollection<EntityType>*> : public PortConverterCollection<const gxeng::EntityCollection<EntityType>*> {
public:
	PortConverter() :
		PortConverterCollection<const gxeng::EntityCollection<EntityType>*>(&FromScene) {}

protected:
	static const gxeng::EntityCollection<EntityType>* FromScene(const gxeng::Scene* scene) {
		return &scene->GetEntities<EntityType>();
	}
};



} // namespace inl