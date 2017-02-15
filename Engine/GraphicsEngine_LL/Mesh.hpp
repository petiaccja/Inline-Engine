#pragma once

#include "MeshBuffer.hpp"
#include "Vertex.hpp"

#include <type_traits>


namespace inl {
namespace gxeng {


class Mesh : protected MeshBuffer {
public:
	struct Element {
		eVertexElementSemantic semantic;
		int index;
		int offset;
	};
public:
	Mesh(MemoryManager* memoryManager) : MeshBuffer(memoryManager) {}

	void Set(const VertexBase* vertices, size_t numVertices, const unsigned* indices, size_t numIndices);
	void Update(const VertexBase* vertices, size_t numVertices, size_t offsetInVertices);
	void Clear();

	using MeshBuffer::GetNumStreams;
	using MeshBuffer::GetVertexBuffer;
	using MeshBuffer::GetVertexBufferStride;
	using MeshBuffer::GetIndexBuffer;
	using MeshBuffer::IsIndexBuffer32Bit;

	const std::vector<Element>& GetVertexBufferElements(size_t streamIndex) const;

	bool EqualElements(const Mesh& rhs) const;
	bool EqualLayout(const Mesh& rhs) const;
	size_t GetElementHash() const;
	size_t GetLayoutHash() const;
private:
	static void CalculateHashes(const std::vector<std::vector<Element>>& layout, size_t& elementHash, size_t& layoutHash);
	static std::vector<Element> GetAllElements(const std::vector<std::vector<Element>>& layout);
	static void RadixSortElements(std::vector<Element>& elements);
private:
	std::vector<std::vector<Element>> m_layout;
	size_t m_elementHash = 0;
	size_t m_layoutHash = 0;
};



} // namespace gxeng
} // namespace inl