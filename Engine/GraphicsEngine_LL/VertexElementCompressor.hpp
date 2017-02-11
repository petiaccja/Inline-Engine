#pragma once

#include "Vertex.hpp"
#include <array>


namespace inl {
namespace gxeng {

template <eVertexElementSemantic Semantic>
class VertexElementCompressor {
public:
	//static_assert(false, "VertexElement compressor must be specialized for given semantics one-by-one.");
};


template <>
class VertexElementCompressor<eVertexElementSemantic::POSITION> {
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


template <>
class VertexElementCompressor<eVertexElementSemantic::TEX_COORD> {
public:
	static size_t Size() { return 2 * sizeof(float); }

	static std::array<uint8_t, 2 * sizeof(float)> Compress(const mathfu::Vector<float, 2>& input) {
		std::array<uint8_t, 2 * sizeof(float)> ret;
		*reinterpret_cast<uint32_t*>(ret.data() + 0) = *reinterpret_cast<const uint32_t*>(&input.x());
		*reinterpret_cast<uint32_t*>(ret.data() + 4) = *reinterpret_cast<const uint32_t*>(&input.y());
		return ret;
	}

	static mathfu::Vector<float, 2> Decompress(const void* input) {
		mathfu::Vector<float, 2> ret;
		ret.x() = *reinterpret_cast<const float*>(input) + 0;
		ret.y() = *reinterpret_cast<const float*>(input) + 1;
		return ret;
	}
};


template <>
class VertexElementCompressor<eVertexElementSemantic::COLOR> {
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
	struct Element {
		eVertexElementSemantic semantic;
		int index;
		int offset;
	};
public:
	static size_t Size(const VertexBase& input, const std::vector<bool>& elementMap) {
		size_t size = 0;
		int index = 0;

		for (auto& element : input.GetElements()) {
			if (index < elementMap.size() && elementMap[index]) {
				switch (element.semantic) {
					case eVertexElementSemantic::POSITION:
						size += VertexElementCompressor<eVertexElementSemantic::POSITION>::Size();
						break;
					case eVertexElementSemantic::NORMAL:
						size += VertexElementCompressor<eVertexElementSemantic::NORMAL>::Size();
						break;
					case eVertexElementSemantic::TEX_COORD:
						size += VertexElementCompressor<eVertexElementSemantic::TEX_COORD>::Size();
						break;
					case eVertexElementSemantic::COLOR:
						size += VertexElementCompressor<eVertexElementSemantic::COLOR>::Size();
						break;
					default:
						throw std::domain_error("Unsupported vertex element type.");
						break;
				}
			}

			++index;
		}
		return size;
	}

	static std::vector<Element> Compress(const VertexBase& input, const std::vector<bool>& elementMap, void* output) {
		size_t offset = 0;
		int index = 0;
		uint8_t* outputPtr = reinterpret_cast<uint8_t*>(output);
		std::vector<Element> compressedElements;

		for (auto& element : input.GetElements()) {
			if (elementMap.size() > index && (bool)elementMap[index]) {
				compressedElements.push_back({ element.semantic, element.index, (int)offset });

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
						auto compressed = VertexElementCompressor<eVertexElementSemantic::NORMAL>::Compress(
							dynamic_cast<const VertexPart<eVertexElementSemantic::NORMAL>&>(input).GetNormal(element.index));

						for (auto v : compressed) {
							*outputPtr = v;
							++outputPtr;
						}

						offset += VertexElementCompressor<eVertexElementSemantic::NORMAL>::Size();
						break;
					}
					case eVertexElementSemantic::TEX_COORD:
					{
						auto compressed = VertexElementCompressor<eVertexElementSemantic::TEX_COORD>::Compress(
							dynamic_cast<const VertexPart<eVertexElementSemantic::TEX_COORD>&>(input).GetTexCoord(element.index));

						for (auto v : compressed) {
							*outputPtr = v;
							++outputPtr;
						}

						offset += VertexElementCompressor<eVertexElementSemantic::NORMAL>::Size();
						break;
					}
					case eVertexElementSemantic::COLOR:
					{
						auto compressed = VertexElementCompressor<eVertexElementSemantic::COLOR>::Compress(
							dynamic_cast<const VertexPart<eVertexElementSemantic::COLOR>&>(input).GetColor(element.index));

						for (auto v : compressed) {
							*outputPtr = v;
							++outputPtr;
						}

						offset += VertexElementCompressor<eVertexElementSemantic::COLOR>::Size();
						break;
					}
					default:
						throw std::domain_error("Unsupported element type.");
						break;
				}
			}

			++index;
		}

		return compressedElements;
	}
};



} // namespace gxeng
} // namespace inl