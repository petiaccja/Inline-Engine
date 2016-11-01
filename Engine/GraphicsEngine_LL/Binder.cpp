#pragma once

#include <GraphicsApi_LL/IGraphicsApi.hpp>

#include "Binder.hpp"
#include <algorithm>


namespace inl {
namespace gxeng {



Binder::Binder(inl::gxapi::IGraphicsApi* gxApi, const std::vector<BindParameterDesc>& parameters, const std::vector<gxapi::StaticSamplerDesc>& staticSamplers) {
	CalculateLayout(parameters);
	m_rootSignatureDesc.staticSamplers = staticSamplers;
	m_rootSignature.reset(gxApi->CreateRootSignature(m_rootSignatureDesc));
}

void Binder::Translate(BindParameter parameter, int & rootParamIndex, int & rootTableIndex) const {
	auto result = FindMapping(parameter);
	if (result.second != true) {
		throw std::out_of_range("Parameter was not found.");
	}
	rootParamIndex = result.first->rootParamIndex;
	rootTableIndex = result.first->rootTableIndex;
}


void Binder::CalculateLayout(const std::vector<BindParameterDesc>& parameters) {
	// copy input parameters to mapping
	m_parameters.reserve(parameters.size());

	std::vector<std::vector<BindParameterDesc>> tableParams; // goes into descriptor heap (scratch space)
	std::vector<BindParameterDesc> samplerParams; // samplers are treated separately
	std::vector<BindParameterDesc> constantParams; // inline constants and inline CBVs

	// put parameters to the right slot/table in the root signature
	DistributeParameters(parameters, tableParams, samplerParams, constantParams);

	// declare root signature desc
	gxapi::RootSignatureDesc desc;

	// copy parameters to m_parameters and fill root signature desc
	int rootParamIndex = 0;
	int rootTableIndex = 0;
	// constants
	for (const auto& param : constantParams) {
		// copy mapping
		RootParameterMapping mapping;
		mapping.bindParam = param.parameter;
		mapping.rootParamIndex = rootParamIndex++;
		mapping.constantCount = (param.constantSize + 3) / 4;
		m_parameters.push_back(mapping);

		// fill desc
		if (mapping.constantCount > 0) {
			desc.rootParameters.push_back(gxapi::RootParameterDesc::Constant(
				mapping.constantCount,
				mapping.bindParam.reg,
				mapping.bindParam.space,
				param.shaderVisibility));
		}
		else {
			desc.rootParameters.push_back(gxapi::RootParameterDesc::Cbv(
				mapping.bindParam.reg,
				mapping.bindParam.space,
				param.shaderVisibility));
		}
	}
	// descriptor tables
	for (auto& table : tableParams) {
		rootTableIndex = 0;

		// sort table by type (CBV/SRV/UAV) by register by space [radix sort]
		std::stable_sort(table.begin(), table.end(), [](const BindParameterDesc& lhs, const BindParameterDesc& rhs)
		{
			return lhs.parameter.reg < rhs.parameter.reg;
		});
		std::stable_sort(table.begin(), table.end(), [](const BindParameterDesc& lhs, const BindParameterDesc& rhs)
		{
			return lhs.parameter.space < rhs.parameter.space;
		});
		std::stable_sort(table.begin(), table.end(), [](const BindParameterDesc& lhs, const BindParameterDesc& rhs)
		{
			return lhs.parameter.type < rhs.parameter.type;
		});

		// fill desc
		desc.rootParameters.push_back(gxapi::RootParameterDesc::DescriptorTable());
		auto& rootTable = desc.rootParameters[rootParamIndex].As<gxapi::RootParameterDesc::DESCRIPTOR_TABLE>();

		int typeStreakLength = 0; // how many of the same type (cbv/srv/uav) do we encounter in a row
		eBindParameterType typeOfStreak = eBindParameterType::CONSTANT;
		int spaceOfStreak = 0;
		int regOfStreak = 0;
		for (const auto& param : table) {
			// copy mapping
			RootParameterMapping mapping;
			mapping.bindParam = param.parameter;
			mapping.rootParamIndex = rootParamIndex;
			mapping.rootTableIndex = rootTableIndex++;
			mapping.constantCount = 0;
			m_parameters.push_back(mapping);

			// fill desc
			// start new streak if needed
			if (param.parameter.type != typeOfStreak
				|| param.parameter.space != spaceOfStreak
				|| param.parameter.reg != regOfStreak + 1
				|| typeStreakLength == 0)
			{
				rootTable.ranges.push_back(gxapi::DescriptorRange{ CastRangeType(param.parameter.type), 0, param.parameter.reg, param.parameter.space });
			}

			typeOfStreak = param.parameter.type;
			regOfStreak = param.parameter.reg;
			rootTable.ranges.back().numDescriptors++;
		}

		++rootParamIndex;
	}

	// radix sort mapping for easy search
	std::sort(m_parameters.begin(), m_parameters.end(), [](const RootParameterMapping& lhs, const RootParameterMapping& rhs)
	{
		return RadixLess(lhs.bindParam, rhs.bindParam);
	});

	m_rootSignatureDesc = desc;
}


// Note that this function is optimized for nvidia, without taking much care for nvidia either.
void Binder::DistributeParameters(const std::vector<BindParameterDesc>& parameters,
								  std::vector<std::vector<BindParameterDesc>> & tableParams,
								  std::vector<BindParameterDesc> & samplerParams,
								  std::vector<BindParameterDesc> & constantParams)
{
	// put SRV's and UAV's into descriptor table: they have so many limitation that inlining them is basically worthless
	// put samplers into separate list
	for (const auto& param : parameters) {
		if (param.parameter.type == eBindParameterType::TEXTURE || param.parameter.type == eBindParameterType::UNORDERED) {
			if (tableParams.size() == 0) {
				tableParams.resize(1);
			}
			tableParams[0].push_back(param);
		}
		else if (param.parameter.type == eBindParameterType::SAMPLER) {
			samplerParams.push_back(param);
		}
		else {
			constantParams.push_back(param);
		}
	}

	// how to compute required number of bytes in root signature
	auto usedSpace = [&] {
		int size = 0;
		//for (const auto& param : parameters) {
		//	size += ((param.constantSize + 3) / 4 > 0 ? param.constantSize : 8); break; // inline constant size OR 64 bit virtual address
		//}
		//size += tableParams.empty() ? 0 : 4; // 32 bits for each descriptor table

		for (const auto& table : tableParams) {
			size += 4;
		}
		for (const auto& constant : constantParams) {
			size += constant.constantSize > 0 ? ((constant.constantSize + 3) / 4 * 4) : 8;
		}

		return size;
	};


	// sort constant parametes by size, ascending, 0 is considered biggest
	std::sort(constantParams.begin(), constantParams.end(), [](const BindParameterDesc& lhs, const BindParameterDesc& rhs)
	{
		auto lhscmp = lhs.constantSize <= 0 ? std::numeric_limits<decltype(lhs.constantSize)>::max() : lhs.constantSize;
		auto rhscmp = rhs.constantSize <= 0 ? std::numeric_limits<decltype(rhs.constantSize)>::max() : rhs.constantSize;
		return lhscmp < rhscmp;
	});

	// sort constant parametes by change frequency
	auto largeBegin = constantParams.begin();
	while (largeBegin != constantParams.end() && largeBegin->constantSize <= 8) {
		largeBegin++;
	}

	std::sort(largeBegin, constantParams.end(), [](const BindParameterDesc& lhs, const BindParameterDesc& rhs)
	{
		return lhs.relativeChangeFrequency < rhs.relativeChangeFrequency;
	});

	// convert inline constants to CBVs to save space
	{
		auto convertIt = constantParams.rbegin();
		auto endIt = constantParams.rend();
		while (maxSize < usedSpace() && convertIt != endIt) {
			convertIt->constantSize = 0;
			++convertIt;
		}
	}

	// throw CBVs into descriptor table until there's enough space
	{
		while (maxSize < usedSpace()) {
			auto constantIt = constantParams.rbegin();
			auto endIt = constantParams.rend();
			if (constantIt != endIt) {
				if (tableParams.size() == 0) {
					tableParams.resize(1);
				}
				tableParams[0].push_back(*constantIt);
				constantParams.erase(--constantIt.base());
			}
		}
	}
}


std::pair<std::vector<Binder::RootParameterMapping>::const_iterator, bool> Binder::FindMapping(BindParameter param) const {
	RootParameterMapping key;
	key.bindParam = param;
	key.constantCount = 0;
	auto it = std::lower_bound(m_parameters.begin(),
							   m_parameters.end(),
							   key,
							   [](const RootParameterMapping& lhs, const RootParameterMapping& rhs) {
		return RadixLess(lhs.bindParam, rhs.bindParam);
	});
	return{ it, it != m_parameters.end() };
}



bool Binder::RadixLess(const BindParameter& lhs, const BindParameter& rhs) {
	if (lhs.type < rhs.type) {
		return true; // less
	}
	if (lhs.type == rhs.type && lhs.space < rhs.space) {
		return true; // less
	}
	if (lhs.type == rhs.type && lhs.space == rhs.space && rhs.reg < rhs.space) {
		return true; // less
	}
	return false; // greater-equal
}


gxapi::DescriptorRange::eType Binder::CastRangeType(eBindParameterType source) {
	switch (source) {
		case eBindParameterType::CONSTANT: return gxapi::DescriptorRange::CBV;
		case eBindParameterType::TEXTURE: return gxapi::DescriptorRange::SRV;
		case eBindParameterType::UNORDERED: return gxapi::DescriptorRange::UAV;
		case eBindParameterType::SAMPLER: return gxapi::DescriptorRange::SAMPLER;
	}
	assert(false);
	return gxapi::DescriptorRange::eType(0); // just to silence the fucking warning
}


} // namespace gxeng
} // namespace inl
