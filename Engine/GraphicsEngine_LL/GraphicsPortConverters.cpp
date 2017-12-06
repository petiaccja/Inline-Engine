#include "GraphicsPortConverters.hpp"


namespace inl {


// eFormat

std::string PortConverter<gxapi::eFormat>::ToString(const gxapi::eFormat& fmt) const {
	const std::unordered_set<const Record*, FmtHash, FmtHash>* byFmt;
	const std::unordered_set<const Record*, StrHash, StrHash>* byStr;

	GetMaps(&byFmt, &byStr);

	Record key{ {}, fmt };
	auto it = byFmt->find(&key);
	if (it != byFmt->end()) {
		return (*it)->first;
	}
	else {
		throw OutOfRangeException("Could not translate format. Update table.");
	}
}

gxapi::eFormat PortConverter<gxapi::eFormat>::FromString(const std::string& str) {
	const std::unordered_set<const Record*, FmtHash, FmtHash>* byFmt;
	const std::unordered_set<const Record*, StrHash, StrHash>* byStr;

	GetMaps(&byFmt, &byStr);

	Record key{ str, gxapi::eFormat::UNKNOWN };
	auto it = byStr->find(&key);
	if (it != byStr->end()) {
		return (*it)->second;
	}
	else {
		throw OutOfRangeException("Could not translate string. Update table or user error.");
	}
}

void PortConverter<gxapi::eFormat>::GetMaps(const std::unordered_set<const Record*, FmtHash, FmtHash>** byFmt,
	const std::unordered_set<const Record*, StrHash, StrHash>** byStr)
{
	static std::vector<Record> records = {
		{ "UNKNOWN",gxapi::eFormat::UNKNOWN },

		{ "R32G32B32A32_TYPELESS",gxapi::eFormat::R32G32B32A32_TYPELESS },
		{ "R32G32B32A32_FLOAT",gxapi::eFormat::R32G32B32A32_FLOAT },
		{ "R32G32B32A32_UINT",gxapi::eFormat::R32G32B32A32_UINT },
		{ "R32G32B32A32_SINT",gxapi::eFormat::R32G32B32A32_SINT },

		{ "R32G32B32_TYPELESS",gxapi::eFormat::R32G32B32_TYPELESS },
		{ "R32G32B32_FLOAT",gxapi::eFormat::R32G32B32_FLOAT },
		{ "R32G32B32_UINT",gxapi::eFormat::R32G32B32_UINT },
		{ "R32G32B32_SINT",gxapi::eFormat::R32G32B32_SINT },

		{ "R16G16B16A16_TYPELESS",gxapi::eFormat::R16G16B16A16_TYPELESS },
		{ "R16G16B16A16_FLOAT",gxapi::eFormat::R16G16B16A16_FLOAT },
		{ "R16G16B16A16_UNORM",gxapi::eFormat::R16G16B16A16_UNORM },
		{ "R16G16B16A16_UINT",gxapi::eFormat::R16G16B16A16_UINT },
		{ "R16G16B16A16_SNORM",gxapi::eFormat::R16G16B16A16_SNORM },
		{ "R16G16B16A16_SINT",gxapi::eFormat::R16G16B16A16_SINT },

		{ "R32G32_TYPELESS",gxapi::eFormat::R32G32_TYPELESS },
		{ "R32G32_FLOAT",gxapi::eFormat::R32G32_FLOAT },
		{ "R32G32_UINT",gxapi::eFormat::R32G32_UINT },
		{ "R32G32_SINT",gxapi::eFormat::R32G32_SINT },

		{ "R32G8X24_TYPELESS",gxapi::eFormat::R32G8X24_TYPELESS },
		{ "D32_FLOAT_S8X24_UINT",gxapi::eFormat::D32_FLOAT_S8X24_UINT },
		{ "R32_FLOAT_X8X24_TYPELESS",gxapi::eFormat::R32_FLOAT_X8X24_TYPELESS },
		{ "X32_TYPELESS_G8X24_UINT",gxapi::eFormat::X32_TYPELESS_G8X24_UINT },

		{ "R10G10B10A2_TYPELESS",gxapi::eFormat::R10G10B10A2_TYPELESS },
		{ "R10G10B10A2_UNORM",gxapi::eFormat::R10G10B10A2_UNORM },
		{ "R10G10B10A2_UINT",gxapi::eFormat::R10G10B10A2_UINT },
		{ "R11G11B10_FLOAT",gxapi::eFormat::R11G11B10_FLOAT },

		{ "R8G8B8A8_TYPELESS",gxapi::eFormat::R8G8B8A8_TYPELESS },
		{ "R8G8B8A8_UNORM",gxapi::eFormat::R8G8B8A8_UNORM },
		{ "R8G8B8A8_UNORM_SRGB",gxapi::eFormat::R8G8B8A8_UNORM_SRGB },
		{ "R8G8B8A8_UINT",gxapi::eFormat::R8G8B8A8_UINT },
		{ "R8G8B8A8_SNORM",gxapi::eFormat::R8G8B8A8_SNORM },
		{ "R8G8B8A8_SINT",gxapi::eFormat::R8G8B8A8_SINT },

		{ "R16G16_TYPELESS",gxapi::eFormat::R16G16_TYPELESS },
		{ "R16G16_FLOAT",gxapi::eFormat::R16G16_FLOAT },
		{ "R16G16_UNORM",gxapi::eFormat::R16G16_UNORM },
		{ "R16G16_UINT",gxapi::eFormat::R16G16_UINT },
		{ "R16G16_SNORM",gxapi::eFormat::R16G16_SNORM },
		{ "R16G16_SINT",gxapi::eFormat::R16G16_SINT },

		{ "R32_TYPELESS",gxapi::eFormat::R32_TYPELESS },
		{ "D32_FLOAT",gxapi::eFormat::D32_FLOAT },
		{ "R32_FLOAT",gxapi::eFormat::R32_FLOAT },
		{ "R32_UINT",gxapi::eFormat::R32_UINT },
		{ "R32_SINT",gxapi::eFormat::R32_SINT },

		{ "R24G8_TYPELESS",gxapi::eFormat::R24G8_TYPELESS },
		{ "D24_UNORM_S8_UINT",gxapi::eFormat::D24_UNORM_S8_UINT },
		{ "R24_UNORM_X8_TYPELESS",gxapi::eFormat::R24_UNORM_X8_TYPELESS },
		{ "X24_TYPELESS_G8_UINT",gxapi::eFormat::X24_TYPELESS_G8_UINT },

		{ "R8G8_TYPELESS",gxapi::eFormat::R8G8_TYPELESS },
		{ "R8G8_UNORM",gxapi::eFormat::R8G8_UNORM },
		{ "R8G8_UINT",gxapi::eFormat::R8G8_UINT },
		{ "R8G8_SNORM",gxapi::eFormat::R8G8_SNORM },
		{ "R8G8_SINT",gxapi::eFormat::R8G8_SINT },

		{ "R16_TYPELESS",gxapi::eFormat::R16_TYPELESS },
		{ "R16_FLOAT",gxapi::eFormat::R16_FLOAT },
		{ "D16_UNORM",gxapi::eFormat::D16_UNORM },
		{ "R16_UNORM",gxapi::eFormat::R16_UNORM },
		{ "R16_UINT",gxapi::eFormat::R16_UINT },
		{ "R16_SNORM",gxapi::eFormat::R16_SNORM },
		{ "R16_SINT",gxapi::eFormat::R16_SINT },

		{ "R8_TYPELESS",gxapi::eFormat::R8_TYPELESS },
		{ "R8_UNORM",gxapi::eFormat::R8_UNORM },
		{ "R8_UINT",gxapi::eFormat::R8_UINT },
		{ "R8_SNORM",gxapi::eFormat::R8_SNORM },
		{ "R8_SINT",gxapi::eFormat::R8_SINT },
		{ "A8_UNORM",gxapi::eFormat::A8_UNORM },
	};

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



std::string PortConverter<gxapi::RenderTargetBlendState>::ToString(const gxapi::RenderTargetBlendState& fmt) const {
	//throw NotImplementedException("No, peter, you're gonna have to implement this. Boring... /cry");
	return "yay";
}
gxapi::RenderTargetBlendState PortConverter<gxapi::RenderTargetBlendState>::FromString(const std::string&) {
	//throw NotImplementedException("No, peter, you're gonna have to implement this. Boring... /cry");
	return {};
}



std::string PortConverter<gxeng::TextureUsage>::ToString(const gxeng::TextureUsage& fmt) const {
	//throw NotImplementedException("No, peter, you're gonna have to implement this. Boring... /cry");
	return "yay";
}
gxeng::TextureUsage PortConverter<gxeng::TextureUsage>::FromString(const std::string&) {
	//throw NotImplementedException("No, peter, you're gonna have to implement this. Boring... /cry");
	return {};
}




};