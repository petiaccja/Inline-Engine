#include "Mesh.hpp"
#include "VertexElementCompressor.hpp"
#include <BaseLibrary/ArrayView.hpp>

using exc::ArrayView;


namespace inl {
namespace gxeng {



void Mesh::Set(const VertexBase* vertices, size_t numVertices, const unsigned* indices, size_t numIndices) {
	// Create constants
	auto& elements = vertices[0].GetElements();
	std::vector<bool> elementMap(elements.size(), true);
	uint32_t compressedStride = (uint32_t)VertexCompressor::Size(*vertices, elementMap);

	// Create a view to iterate over vertices
	ArrayView<const VertexBase> inputArrayView{ vertices, numVertices, vertices->StructureSize() };

	// Create buffer for compressed vertices of stream
	std::unique_ptr<uint8_t[]> compressedData = std::make_unique<uint8_t[]>(compressedStride * numVertices);

	// Fill stream
	ArrayView<VertexBase> outputArrayView{ reinterpret_cast<VertexBase*>(compressedData.get()), numVertices, compressedStride };
	std::vector<VertexCompressor::Element> compressedElements;
	for (size_t i = 0; i < numVertices; i++) {
		compressedElements = VertexCompressor::Compress(inputArrayView[i], elementMap, &outputArrayView[i]);
	}

	// Set data
	VertexStream stream;
	stream.stride = compressedStride;
	stream.count = numVertices;
	stream.data = compressedData.get();
	MeshBuffer::Set(&stream, &stream + 1, indices, indices + numIndices);

	// Set stream elements.
	m_layout.clear();
	std::vector<Element> streamElements;
	for (const auto& e : compressedElements) {
		streamElements.push_back({ e.semantic, e.index, e.offset });
	}
	m_layout.push_back(streamElements);

	// Calculate hashes
	CalculateHashes(m_layout, m_elementHash, m_layoutHash);
}


void Mesh::Update(const VertexBase* vertices, size_t numVertices, size_t offsetInVertices) {
	// Create constants
	auto& elements = vertices[0].GetElements();
	std::vector<bool> elementMap(elements.size(), true);
	size_t compressedStride = VertexCompressor::Size(*vertices, elementMap);

	// Create a view to iterate over vertices
	ArrayView<const VertexBase> inputArrayView{ vertices, numVertices, vertices->StructureSize() };

	// Create a stream
	std::unique_ptr<uint8_t[]> compressedData = std::make_unique<uint8_t[]>(compressedStride * numVertices);

	// Fill stream
	ArrayView<VertexBase> outputArrayView{ reinterpret_cast<VertexBase*>(compressedData.get()), numVertices, compressedStride };
	for (size_t i = 0; i < numVertices; i++) {
		VertexCompressor::Compress(inputArrayView[i], elementMap, &outputArrayView[i]);
	}

	// Update data
	MeshBuffer::Update(0, compressedData.get(), numVertices, offsetInVertices);
}


void Mesh::Clear() {
	MeshBuffer::Clear();
	m_layout.clear();
}


const std::vector<Mesh::Element>& Mesh::GetVertexBufferElements(size_t streamIndex) const {
	assert(streamIndex < GetNumStreams());
	return m_layout[streamIndex];
}



bool Mesh::EqualElements(const Mesh& rhs) const {
	if (m_elementHash != rhs.m_elementHash) {
		return false;
	}

	auto lhsElements = GetAllElements(m_layout);
	auto rhsElements = GetAllElements(rhs.m_layout);
	if (lhsElements.size() != rhsElements.size()) {
		return false;
	}

	RadixSortElements(lhsElements);
	RadixSortElements(rhsElements);
	for (size_t i = 0; i < lhsElements.size(); ++i) {
		if (lhsElements[i].semantic != rhsElements[i].semantic
			|| lhsElements[i].index != rhsElements[i].index
			|| lhsElements[i].offset != rhsElements[i].offset)
		{
			return false;
		}
	}

	return true;
}

bool Mesh::EqualLayout(const Mesh& rhs) const {
	if (m_layoutHash != rhs.m_layoutHash) {
		return false;
	}

	// same as above, except for sorting
	auto lhsElements = GetAllElements(m_layout);
	auto rhsElements = GetAllElements(rhs.m_layout);
	if (lhsElements.size() != rhsElements.size()) {
		return false;
	}
	for (size_t i = 0; i < lhsElements.size(); ++i) {
		if (lhsElements[i].semantic != rhsElements[i].semantic
			|| lhsElements[i].index != rhsElements[i].index
			|| lhsElements[i].offset != rhsElements[i].offset)
		{
			return false;
		}
	}

	return true;
}

size_t Mesh::GetElementHash() const {
	return m_elementHash;
}

size_t Mesh::GetLayoutHash() const {
	return m_layoutHash;
}


// source for hashes: http://www.tommyds.it/doc/tommyhash_8h_source.html
inline uint32_t inthash(uint32_t key) {
	key -= key << 6;
	key ^= key >> 17;
	key -= key << 9;
	key ^= key << 4;
	key -= key << 3;
	key ^= key << 10;
	key ^= key >> 15;

	return key;
}
inline uint64_t inthash(uint64_t key) {
	key = ~key + (key << 21);
	key = key ^ (key >> 24);
	key = key + (key << 3) + (key << 8);
	key = key ^ (key >> 14);
	key = key + (key << 2) + (key << 4);
	key = key ^ (key >> 28);
	key = key + (key << 31);

	return key;
}


std::vector<Mesh::Element> Mesh::GetAllElements(const std::vector<std::vector<Element>>& layout) {
	std::vector<Element> allElements;
	std::vector<Element> streamElements;

	for (const auto& s : layout) {
		streamElements = s;
		// sort elements by offset (elements are unique by offset, no radix sort needed)
		// sorting is needed because different order still means the same stream content layout
		// this way, hash value will be the same regardless of order; only content layout matters
		std::sort(streamElements.begin(), streamElements.end(), [](const Element& lhs, const Element& rhs) {
			return lhs.offset < rhs.offset;
		});
		// push list to all elements
		for (const auto& e : streamElements) {
			allElements.push_back(e);
		}
	}

	return allElements;
}


void Mesh::RadixSortElements(std::vector<Element>& elements) {
	std::stable_sort(elements.begin(), elements.end(), [](const Element& lhs, const Element& rhs) {
		return lhs.offset < rhs.offset;
	});
	std::stable_sort(elements.begin(), elements.end(), [](const Element& lhs, const Element& rhs) {
		return lhs.index < rhs.index;
	});
	std::stable_sort(elements.begin(), elements.end(), [](const Element& lhs, const Element& rhs) {
		return lhs.semantic < rhs.semantic;
	});
}


void Mesh::CalculateHashes(const std::vector<std::vector<Element>>& layout, size_t& elementHash, size_t& layoutHash) {
	std::vector<Element> allElements;

	elementHash = 0;
	layoutHash = 0;

	allElements = GetAllElements(layout);

	// hashing allElements will result in a hash tied to a specific layout
	// this is because allElements retains the information as to which element is in which stream
	for (const auto& e : allElements) {
		layoutHash ^= inthash((size_t)e.semantic);
		layoutHash ^= inthash((size_t)e.index);
		layoutHash ^= inthash((size_t)e.offset);
	}

	// now we order allElements to remove layout information, and keep only element information
	RadixSortElements(allElements);

	for (const auto& e : allElements) {
		elementHash ^= inthash((size_t)e.semantic);
		elementHash ^= inthash((size_t)e.index);
		elementHash ^= inthash((size_t)e.offset);
	}
}


} // namespace gxeng
} // namespace inl