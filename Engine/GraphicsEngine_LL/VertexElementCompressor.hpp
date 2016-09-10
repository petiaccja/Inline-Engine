#pragma once

#include "Vertex.hpp"
#include <array>


namespace inl {
namespace gxeng {

template <eVertexElementSemantic Semantic>
class VertexElementCompressor {
public:
};


template <>
class VertexElementCompressor<eVertexElementSemantic::POSITION> {
public:
	static size_t Size() { return 3 * sizeof(float); }

	static std::array<uint8_t, 3*sizeof(float)> Compress(const mathfu::Vector<float, 3>& input) {
		std::array<uint8_t, 3 * sizeof(float)> ret;
		*reinterpret_cast<uint32_t*>(ret.data() + 0) = *reinterpret_cast<const uint32_t*>(&input.x());
		*reinterpret_cast<uint32_t*>(ret.data() + 4) = *reinterpret_cast<const uint32_t*>(&input.y());
		*reinterpret_cast<uint32_t*>(ret.data() + 8) = *reinterpret_cast<const uint32_t*>(&input.z());
		return ret;
	}

	static mathfu::Vector<float, 3> Decompress(const void* input) {
		mathfu::Vector<float, 3> ret;
		ret.x() = *reinterpret_cast<const float*>(input) + 0;
		ret.y() = *reinterpret_cast<const float*>(input) + 1;
		ret.z() = *reinterpret_cast<const float*>(input) + 2;
		return ret;
	}
};


template <>
class VertexElementCompressor<eVertexElementSemantic::NORMAL> {
public:
	static size_t Size() { return 3 * sizeof(float); }

	static std::array<uint8_t, 3 * sizeof(float)> Compress(const mathfu::Vector<float, 3>& input) {
		std::array<uint8_t, 3 * sizeof(float)> ret;
		*reinterpret_cast<uint32_t*>(ret.data() + 0) = *reinterpret_cast<const uint32_t*>(&input.x());
		*reinterpret_cast<uint32_t*>(ret.data() + 4) = *reinterpret_cast<const uint32_t*>(&input.y());
		*reinterpret_cast<uint32_t*>(ret.data() + 8) = *reinterpret_cast<const uint32_t*>(&input.z());
		return ret;
	}

	static mathfu::Vector<float, 3> Decompress(const void* input) {
		mathfu::Vector<float, 3> ret;
		ret.x() = *reinterpret_cast<const float*>(input) + 0;
		ret.y() = *reinterpret_cast<const float*>(input) + 1;
		ret.z() = *reinterpret_cast<const float*>(input) + 2;
		return ret;
	}
};


class VertexCompressor {
public:
	static size_t Size(const VertexBase& input, const std::vector<bool>& elementMap) {
		size_t size = 0;
		int index = 0;

		for (auto& element : input.GetElements()) {
			if (elementMap.size() < index && elementMap[index]) {
				switch (element.semantic) {
					case eVertexElementSemantic::POSITION:
						size += VertexElementCompressor<eVertexElementSemantic::POSITION>::Size();
						break;
					case eVertexElementSemantic::NORMAL:
						size += VertexElementCompressor<eVertexElementSemantic::POSITION>::Size();
						break;
					default:
						throw std::domain_error("Unsupported element type.");
						break;
				}
			}

			++index;
		}
		return size;
	}

	static void Compress(const VertexBase& input, const std::vector<bool>& elementMap, void* output) {
		size_t offset = 0;
		int index = 0;
		uint8_t* outputPtr = reinterpret_cast<uint8_t*>(output);

		for (auto& element : input.GetElements()) {
			if (elementMap.size() < index && elementMap[index]) {
				switch (element.semantic) {
					case eVertexElementSemantic::POSITION:
					{
						auto compressed = VertexElementCompressor<eVertexElementSemantic::POSITION>::Compress(
							dynamic_cast<const VertexPart<eVertexElementSemantic::POSITION>&>(input).GetPosition(element.index));

						for (auto v : compressed) {
							*outputPtr = v;
							++outputPtr;
						}

						offset += VertexElementCompressor<eVertexElementSemantic::POSITION>::Size();
						break;
					}
					case eVertexElementSemantic::NORMAL:
					{
						auto compressed = VertexElementCompressor<eVertexElementSemantic::POSITION>::Compress(
							dynamic_cast<const VertexPart<eVertexElementSemantic::POSITION>&>(input).GetPosition(element.index));

						for (auto v : compressed) {
							*outputPtr = v;
							++outputPtr;
						}
						break;

						offset += VertexElementCompressor<eVertexElementSemantic::POSITION>::Size();
					}
					default:
						throw std::domain_error("Unsupported element type.");
						break;
				}
			}

			++index;
		}
	}
};



} // namespace gxeng
} // namespace inl