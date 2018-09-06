#pragma once

#include "MeshBuffer.hpp"
#include <GraphicsEngine/Resources/Vertex.hpp>
#include <GraphicsEngine/Resources/IMesh.hpp>

#include <type_traits>
#include <mutex>
#include <BaseLibrary/UniqueIdGenerator.hpp>


namespace inl {
namespace gxeng {


class Mesh : public IMesh, protected MeshBuffer {
public:
	struct Element {
		eVertexElementSemantic semantic; // Semantic of the vertex elements.
		int index; // Index of the semantic.
		int offset; // Offset of the element within it's vertex stream, in bytes.
	};

	/// <summary> Describes what vertex elements are contained in the streams of the mesh. </summary>
	struct Layout {
	public:
		Layout() = default;
		Layout(std::vector<std::vector<Element>> layout);

		/// <summary> Returns the number of vertex streams. </summary>
		size_t GetStreamCount() const;

		/// <summary> Returns the <paramref name="idx"/>-th stream's elements. </summary>
		const std::vector<Element>& operator[](size_t idx) const { return m_layout[idx]; }

		/// <summary> Returns true if layouts have the exact same elements, but they may be arranged into streams differently. </summary>
		bool EqualElements(const Layout& rhs) const;

		/// <summary> Returns true if layouts have the exact same elements arranged exactly the same way. </summary>
		bool EqualLayout(const Layout& rhs) const;

		/// <summary> Returns the hash for this particular list of elements. </summary>
		size_t GetElementHash() const;

		/// <summary> Returns the hash for this particular list of elements in this particular arrangement. </summary>
		size_t GetLayoutHash() const;

		/// <summary> Return an ID that is the same for all Layouts that contain the same particular list of elements. 
		///		Does not change throughout program execution. </summary>
		UniqueId GetElementId() const;

		/// <summary> Return an ID that is the same for all Layouts that contain the same particular arrangement of elements. 
		///		Does not change throughout program execution. </summary>
		UniqueId GetLayoutId() const;

		/// <summary> Removes all elements. </summary>
		void Clear();


		struct HashElement {
			size_t operator()(const Layout& obj) const { return obj.GetElementHash(); }
		};
		struct EqualToElement {
			size_t operator()(const Layout& lhs, const Layout& rhs) const { return lhs.EqualElements(rhs); }
		};
		struct HashLayout {
			size_t operator()(const Layout& obj) const { return obj.GetLayoutHash(); }
		};
		struct EqualToLayout {
			size_t operator()(const Layout& lhs, const Layout& rhs) const { return lhs.EqualLayout(rhs); }
		};
	private:
		static void CalculateHashes(const std::vector<std::vector<Element>>& layout, size_t& elementHash, size_t& layoutHash);
		static std::vector<Element> GetAllElements(const std::vector<std::vector<Element>>& layout);
		static void RadixSortElements(std::vector<Element>& elements);

	private:
		std::vector<std::vector<Element>> m_layout; // One set of elements for each vertex buffer.
		size_t m_elementHash = 0;
		size_t m_layoutHash = 0;
		UniqueId m_elementId;
		UniqueId m_layoutId;
		static UniqueIdGenerator<Layout, HashElement, EqualToElement> elementIdGenerator;
		static UniqueIdGenerator<Layout, HashLayout, EqualToLayout> layoutIdGenerator;
		static std::mutex idGeneratorMtx;
	};
public:
	Mesh(MemoryManager* memoryManager) : MeshBuffer(memoryManager) {}

	void Set(const VertexBase* vertices, const IVertexReader* vertexReader, size_t numVertices, const unsigned* indices, size_t numIndices) override;
	void Update(const VertexBase* vertices, const IVertexReader* vertexReader, size_t numVertices, size_t offsetInVertices) override;
	void Clear() override;

	using MeshBuffer::GetNumStreams;
	using MeshBuffer::GetVertexBuffer;
	using MeshBuffer::GetVertexBufferStride;
	using MeshBuffer::GetIndexBuffer;
	using MeshBuffer::IsIndexBuffer32Bit;

	const Layout& GetLayout() const;
private:
	Layout m_layout;
};



} // namespace gxeng
} // namespace inl