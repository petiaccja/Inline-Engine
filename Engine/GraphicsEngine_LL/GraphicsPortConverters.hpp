#pragma once

// This file contains all the port converters related to graphics nodes.
// This is required for loading and exporting JSON pipeline descriptions.


#include <BaseLibrary/Graph/Port.hpp>

#include <unordered_set>

// Graphics API & Engine includes
#include <GraphicsApi_LL/Common.hpp>
#include "NodeContext.hpp"



namespace inl {



// eFormat
template <>
class PortConverter<gxapi::eFormat> : public PortConverterCollection<gxapi::eFormat> {
public:
	PortConverter() :
		PortConverterCollection<gxapi::eFormat>(&FromString) {}

	std::string ToString(const gxapi::eFormat& fmt) const override;
protected:
	static gxapi::eFormat FromString(const std::string&);
	using Record = std::pair<std::string, gxapi::eFormat>;
	struct FmtHash {
		size_t operator()(const Record* obj) const { return std::hash<gxapi::eFormat>()(obj->second); }
		size_t operator()(const Record* lhs, const Record* rhs) const { return lhs->second == rhs->second; }
	};
	struct StrHash {
		size_t operator()(const Record* obj) const { return std::hash<std::string>()(obj->first); }
		size_t operator()(const Record* lhs, const Record* rhs) const { return lhs->first == rhs->first; }
	};
	static void GetMaps(const std::unordered_set<const Record*, FmtHash, FmtHash>** byFmt,
		const std::unordered_set<const Record*, StrHash, StrHash>** byStr);
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


} // namespace inl