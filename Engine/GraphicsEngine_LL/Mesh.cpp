#include "Mesh.hpp"

#include <cassert>



namespace inl {
namespace gxeng {



//------------------------------------------------------------------------------
// Mesh
//------------------------------------------------------------------------------


template <class StreamIt, class IndexIt>
void Mesh::Set(StreamIt firstStream, StreamIt lastStream, IndexIt firstIndex, IndexIt lastIndex) {
	static_assert(std::is_same<VertexStream, decltype(*firstStream)>::value, "Not a VertexStream iterator.");
	static_assert(std::is_integral<decltype(*firstStream)>::value, "Indices must be of integral type.");

	// Validate input data
	eValidationResult valid = Validate(firstStream, lastStream, firstIndex, lastIndex);
	switch (valid) {
		case eValidationResult::VERTEX_COUNT_MISMATCH:
			throw std::invalid_argument("All streams must have the same number of vertices.");
			break;
		case eValidationResult::INDEX_TOO_LARGE:
			throw std::invalid_argument("Indices over-index the vertex buffers.");
			break;
		case eValidationResult::NOT_TRIANGLE:
			throw std::invalid_argument("Index count not divisible by 3. Must be triangles.");
			break;
		case eValidationResult::OK:
			break;
	}

	//Optimize(streams, indices);

	// Create vertex buffers and index buffer
	// TODO...
}



void Mesh::Update(int streamIndex, const void* vertexData, int vertexCount, int offsetInVertex) {
	// Overwrite vertex buffer
	// TODO...
}


void Mesh::Clear() {
	
}


template <class StreamIt, class IndexIt>
Mesh::eValidationResult Mesh::Validate(StreamIt firstStream, StreamIt lastStream, IndexIt firstIndex, IndexIt lastIndex) {
	if (firstStream == lastStream) {
		return eValidationResult::CLEAR;
	}

	auto vertexCount = firstStream->VertexCount();
	for (auto streamIt = firstStream; streamIt != lastStream; ++streamIt) {
		if (streamIt->VertexCount() != vertexCount) {
			return eValidationResult::VERTEX_COUNT_MISMATCH;
		}
	}

	size_t numIndices = 0;
	for (auto indexIt = firstIndex; indexIt != lastIndex; ++indexIt) {
		if (*indexIt >= vertexCount) {
			return eValidationResult::INDEX_TOO_LARGE;
		}
		++numIndices;
	}

	if (numIndices % 3 != 0) {
		return eValidationResult::NOT_TRIANGLE;
	}

	return eValidationResult::OK;
}


void Mesh::Optimize(std::vector<VertexStream>& streams, std::vector<unsigned>& indices) {
	// TODO: implement tipsify or Forsyth's algorithm
	// Forsyth's algorithm is slower, but less sensitive to cache size estimation
}


size_t Mesh::GetNumStreams() const {
	return m_vertexBuffers.size();
}


const VertexBuffer* Mesh::GetVertexBuffer(int streamIndex) const {
	return m_vertexBuffers[streamIndex];
}


const IndexBuffer* Mesh::GetIndexBuffer() const {
	return m_indexBuffer;
}





//------------------------------------------------------------------------------
// VertexStream
//------------------------------------------------------------------------------



VertexStream::VertexStream() {
	m_stride = 0;
	m_count = 0;
}


VertexStream::VertexStream(const VertexStream& rhs) {
	m_count = rhs.m_count;
	m_stride = rhs.m_stride;

	if (m_count > 0) {
		m_data.reset(new uint8_t[m_count * m_stride]);
		memcpy(m_data.get(), rhs.m_data.get(), m_count * m_stride);
	}
}


VertexStream::VertexStream(VertexStream&& rhs) {
	m_count = rhs.m_count;
	m_stride = rhs.m_stride;
	rhs.m_count = 0;
	rhs.m_stride = 0;
	m_data = std::move(rhs.m_data);
}


VertexStream& VertexStream::operator=(const VertexStream& rhs) {
	Clear();
	new (this) VertexStream(rhs);
	return *this;
}


VertexStream& VertexStream::operator=(VertexStream&& rhs) {
	Clear();
	new (this) VertexStream(std::move(rhs));
	return *this;
}


void VertexStream::Set(int stride, int count) {
	assert(count > 0);
	assert(stride > 0);

	m_data.reset(new uint8_t[m_stride * m_count]);
}


void VertexStream::Set(const void* data, int stride, int count) {
	assert(count > 0);
	assert(stride > 0);

	m_stride = stride;
	m_count = count;

	m_data.reset(new uint8_t[m_stride * m_count]);
	memcpy(m_data.get(), data, m_count * m_stride);
}


void VertexStream::Set(std::unique_ptr<uint8_t>&& data, int stride, int count) {
	assert(count > 0);
	assert(stride > 0);

	m_stride = stride;
	m_count = count;

	m_data = std::forward<decltype(data)>(data);
}


void VertexStream::Clear() {
	m_data.reset();
	m_stride = 0;
	m_count = 0;
}


bool VertexStream::IsEmpty() const {
	return m_count == 0;
}






} // namespace gxeng
} // namespace inl