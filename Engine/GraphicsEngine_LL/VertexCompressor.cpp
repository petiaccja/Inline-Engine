#include "VertexCompressor.hpp"
#include <InlineMath.hpp>
#include <BaseLibrary/ArrayView.hpp>

#include <algorithm>


namespace inl::gxeng {



//------------------------------------------------------------------------------
// Semantic compressor implementations
//------------------------------------------------------------------------------


void NormalCompressor::Compress(const void* input, void* output) const {
	using InputT = VertexPartReader<eVertexElementSemantic::NORMAL>::DataType;

	const InputT* in = reinterpret_cast<const InputT*>(input);
	Vec3_Packed* out = reinterpret_cast<Vec3_Packed*>(output);

	out->x = in->x;
	out->y = in->y;
	out->z = in->z;
}
int NormalCompressor::Size() const {
	return sizeof(Vec3_Packed);
}
bool NormalCompressor::IsSupported(eVertexElementSemantic semantic) const {
	return semantic == eVertexElementSemantic::NORMAL
		|| semantic == eVertexElementSemantic::TANGENT
		|| semantic == eVertexElementSemantic::BITANGENT;
}




void ColorCompressor::Compress(const void* input, void* output) const {
	using InputT = VertexPartReader<eVertexElementSemantic::COLOR>::DataType;

	const InputT* in = reinterpret_cast<const InputT*>(input);
	Vec3_Packed* out = reinterpret_cast<Vec3_Packed*>(output);

	out->x = in->x;
	out->y = in->y;
	out->z = in->z;
}
int ColorCompressor::Size() const {
	return sizeof(Vec3_Packed);
}
bool ColorCompressor::IsSupported(eVertexElementSemantic semantic) const {
	return semantic == eVertexElementSemantic::COLOR;
}



void PassthroughCompressor::Setup(eVertexElementSemantic semantic, const IVertexReader* reader) {
	m_stride = (int)reader->GetSize(semantic);
}
void PassthroughCompressor::Compress(const void* input, void* output) const {
	memcpy(output, input, m_stride);
}
int PassthroughCompressor::Size() const {
	return m_stride;
}
bool PassthroughCompressor::IsSupported(eVertexElementSemantic semantic) const {
	return true;
}



//------------------------------------------------------------------------------
// Vertex compressor implementation
//------------------------------------------------------------------------------

VertexCompressor::VertexCompressor(
	const IVertexReader* reader, 
	const std::vector<bool>& elementMap) 
{
	assert(reader != nullptr);
	m_reader = reader;

	// Default compressors.
	CreateDefaultCompressorList();

	// Create a filtered list that only has those elements that should be written to output.
	const std::vector<IVertexReader::Element>& elements = reader->GetElements();
	assert(elements.size() == elementMap.size());

	std::vector<IVertexReader::Element> chosenElements;
	for (int i = 0; i < elements.size(); ++i) {
		if (elementMap[i]) {
			chosenElements.push_back(elements[i]);
		}
	}

	// Sort filtered list by semantic, and by index within same semantic.
	// Sorts are seemingly reversed, that's how it works.
	std::stable_sort(
		chosenElements.begin(),
		chosenElements.end(),
		[](const IVertexReader::Element& lhs, const IVertexReader::Element& rhs) {
		return lhs.index < rhs.index;
	});

	std::stable_sort(
		chosenElements.begin(),
		chosenElements.end(),
		[](const IVertexReader::Element& lhs, const IVertexReader::Element& rhs) {
		return lhs.semantic < rhs.semantic;
	});


	// Fill elements.
	for (auto& v : chosenElements) {
		auto* compressor = AssignCompressor(v);

		if (compressor != nullptr) {
			m_elementsToCompress.push_back({
				v,
				compressor
			});
		}
		else {
			m_passThroughCompressors.push_back(std::make_unique<PassthroughCompressor>());
			m_passThroughCompressors.back()->Setup(v.semantic, reader);
			compressor = m_passThroughCompressors.back().get();
			m_elementsToCompress.push_back({
				v,
				compressor
			});
		}
	}
}


std::vector<uint8_t> VertexCompressor::GetCompressedStream(const VertexBase* vertices, size_t vertexCount) const {
	int stride = 0;
	std::vector<int> sizes;
	for (auto& v : m_elementsToCompress) {
		int s = v.assignedCompressor->Size();
		stride += s;
		sizes.push_back(s);
	}

	std::vector<uint8_t> data;
	data.resize(vertexCount * stride);

	size_t offset = 0;
	size_t numElements = m_elementsToCompress.size();
	ArrayView<const VertexBase> inputArray{ vertices, vertexCount, (size_t)m_reader->GetStride() };
	for (size_t vertex = 0; vertex < vertexCount; ++vertex) {
		for (size_t element = 0; element < numElements; ++element) {
			eVertexElementSemantic semantic = m_elementsToCompress[element].sourceElement.semantic;
			int index = m_elementsToCompress[element].sourceElement.index;
			const void* input = m_reader->GetPointer(inputArray[vertex], semantic, index);

			void* output = data.data() + offset;

			m_elementsToCompress[element].assignedCompressor->Compress(input, output);

			offset += sizes[element];
		}
	}

	return data;
}

int VertexCompressor::GetCompressedStride() const {
	int stride = 0;
	for (auto& v : m_elementsToCompress) {
		stride += v.assignedCompressor->Size();
	}
	return stride;
}

std::vector<int> VertexCompressor::GetCompressedOffsets() const {
	int stride = 0;

	auto& elements = m_reader->GetElements();
	std::vector<int> offsets(elements.size(), -1);

	for (auto& v : m_elementsToCompress) {
		int s = v.assignedCompressor->Size();

		for (size_t i = 0; i < elements.size(); ++i) {
			if (v.sourceElement.semantic == elements[i].semantic
				&& v.sourceElement.index == elements[i].index)
			{
				offsets[i] = stride;
			}
		}

		stride += s;
	}
	return offsets;
}




SemanticCompressor* VertexCompressor::AssignCompressor(const IVertexReader::Element& element) {
	auto it = m_availableCompressors.begin();
	while (it != m_availableCompressors.end()) {
		if ((*it)->IsSupported(element.semantic)) {
			return it->get();
		}
		++it;
	}
	auto passit = m_passThroughCompressors.begin();
	while (passit != m_passThroughCompressors.end()) {
		if ((*passit)->IsSupported(element.semantic)) {
			return passit->get();
		}
		++passit;
	}
	return nullptr;
}


void VertexCompressor::CreateDefaultCompressorList() {
	// Compressors are checked in order.
	// They shouldn't have overlapping capabilities.
	
	m_availableCompressors.push_back(std::make_unique<NormalCompressor>());
	m_availableCompressors.push_back(std::make_unique<ColorCompressor>());	
}




} // namespace inl::gxeng