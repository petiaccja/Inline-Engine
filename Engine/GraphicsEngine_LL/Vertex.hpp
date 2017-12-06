#pragma once

#include <type_traits>
#include <typeindex>
#include <cstdint>
#include <vector>
#include <array>
#include <InlineMath.hpp>
#include <BaseLibrary/Exception/Exception.hpp>


namespace inl::gxeng {


/// <summary>
/// You must tell the engine what the vertex data means by specifying its "semantic".
/// Chose one of the available semantics.
/// </summary>
/// <remarks>
/// To extend the list of semantics, you have to
/// (i) add it to this enumeration
/// (ii) declare the VertexPart by INL_GXENG_VERTEX_PART below in this file
/// </remarks>
enum class eVertexElementSemantic {
	POSITION,
	POSITION2D,
	POSITION4D,
	NORMAL,
	TEX_COORD,
	COLOR,
	TANGENT,
	BITANGENT,
};



/// <summary>
/// Vertices are made up of vertex elements.
/// Each element specifies the semantic and an index. The index is used to
/// tell apart elements of the same semantic type.
/// </summary>
/// <remarks>
/// Use and create shortcuts such as Position&lt;Index&gt; for VertexElement&lt;POSITION, Index&gt;
/// </remarks>
template <eVertexElementSemantic Semantic, int Index>
class VertexElement {
public:
	static constexpr eVertexElementSemantic semantic = Semantic;
	static constexpr int index = Index;
};

template <int Index>
using Position = VertexElement<eVertexElementSemantic::POSITION, Index>;

template <int Index>
using Position2D = VertexElement<eVertexElementSemantic::POSITION2D, Index>;

template <int Index>
using Position4D = VertexElement<eVertexElementSemantic::POSITION4D, Index>;

template <int Index>
using Normal = VertexElement<eVertexElementSemantic::NORMAL, Index>;

template <int Index>
using TexCoord = VertexElement<eVertexElementSemantic::TEX_COORD, Index>;

template <int Index>
using Color = VertexElement<eVertexElementSemantic::COLOR, Index>;

template <int Index>
using Tangent = VertexElement<eVertexElementSemantic::TANGENT, Index>;

template <int Index>
using Bitangent = VertexElement<eVertexElementSemantic::BITANGENT, Index>;




//------------------------------------------------------------------------------
namespace impl {


// Stores values related to a certain semantic. The type of the values and list of indices is specified.
// TODO: optimize for Indices... that form a sequence [0,1,2,3 ...]
// TODO: implement binary search for indices that don't form a sequence
template <class T, int... Indices>
class VertexPartData {
private:
	static constexpr int count = sizeof...(Indices);
	static constexpr int table[count] = { Indices... };
protected:
	T values[count]; //
public:
	T& operator[](int index) {
		for (int lookupIdx = 0; lookupIdx < count; ++lookupIdx) {
			if (table[lookupIdx] == index) {
				return values[lookupIdx];
			}
		}
		throw OutOfRangeException("Index not found.");
	}
	const T& operator[](int index) const {
		for (int lookupIdx = 0; lookupIdx < count; ++lookupIdx) {
			if (table[lookupIdx] == index) {
				return values[lookupIdx];
			}
		}
		throw OutOfRangeException("Index not found.");
	}
};
// Just the definition of statics members
template <class T, int... Indices>
constexpr int VertexPartData<T, Indices...>::table[count];



// List of semantics.
template <eVertexElementSemantic... Semantics>
class SemanticList {};

template <class T, class U>
class ConcatSemanticList {
	static_assert("This specialization is obviously inactive.");
};

template <eVertexElementSemantic... Semantics1, eVertexElementSemantic... Semantics2>
class ConcatSemanticList<SemanticList<Semantics1...>, SemanticList<Semantics2...>> {
public:
	using type = SemanticList<Semantics1..., Semantics2...>;
};



// Decides if a SemanticList contains a given Semantic.
template <eVertexElementSemantic Semantic, class List>
class ContainsSemantic;

template <eVertexElementSemantic Semantic, eVertexElementSemantic First, eVertexElementSemantic... Rest>
class ContainsSemantic<Semantic, SemanticList<First, Rest...>> {
public:
	static constexpr bool value = Semantic == First || ContainsSemantic<Semantic, SemanticList<Rest...>>::value;
};

template <eVertexElementSemantic Semantic>
class ContainsSemantic<Semantic, SemanticList<>> {
public:
	static constexpr bool value = false;
};



// Represents a list of arbitrary types.
template <class... Types>
class TypeList {};

template <class T, class U>
class ConcatTypeList;

template <class... Types1, class... Types2>
class ConcatTypeList<TypeList<Types1...>, TypeList<Types2...>> {
public:
	using type = TypeList<Types1..., Types2...>;
};


// Used to conditionally inherit from.
class EmptyClass {};

} // namespace impl
//------------------------------------------------------------------------------




class VertexBase;

template <eVertexElementSemantic Semantic, int... Indices>
class VertexPart;

class VertexPartReaderBase {
public:
	virtual std::type_index GetType() const = 0;
	virtual const std::vector<int>& GetIndices() const = 0;
	virtual size_t GetSize() const = 0;
	virtual void* GetPointer(VertexBase& obj, int index) const = 0;
	virtual const void* GetPointer(const VertexBase& obj, int index) const = 0;
};

template <eVertexElementSemantic Semantic>
class VertexPartReader;

template <eVertexElementSemantic Semantic, int... Indices>
class VertexPartReaderImpl;


#define INL_GXENG_SIMPLE_ARG(...) __VA_ARGS__
#define INL_GXENG_VERTEX_PART(SEMANTIC, DATA_TYPE, GET_NAME, MULTI_NAME, SINGLE_NAME)		\
template <int Index1, int Index2, int... Indices>											\
class VertexPart<SEMANTIC, Index1, Index2, Indices...> {									\
public:																						\
	using DataType = DATA_TYPE;																\
	impl::VertexPartData<DataType, Index1, Index2, Indices...> MULTI_NAME;					\
};																							\
																							\
template <int Index1>																		\
class VertexPart<SEMANTIC, Index1> {														\
public:																						\
	using DataType = DATA_TYPE;																\
	VertexPart() : SINGLE_NAME() {}															\
	VertexPart(const VertexPart& rhs) : SINGLE_NAME(rhs.SINGLE_NAME) {}						\
	VertexPart& operator=(const VertexPart& rhs) {SINGLE_NAME = rhs.SINGLE_NAME; return *this;}	\
	~VertexPart() { SINGLE_NAME.~DATA_TYPE(); }												\
	union {																					\
		impl::VertexPartData<DataType, Index1> MULTI_NAME;									\
		DataType SINGLE_NAME;																\
	};																						\
};																							\
																							\
template <>																					\
class VertexPartReader<SEMANTIC> : public VertexPartReaderBase {							\
public:																						\
	virtual ~VertexPartReader() {}															\
	using DataType = DATA_TYPE;																\
	virtual DataType& GET_NAME(VertexBase&, int index) const = 0;							\
	virtual const DataType& GET_NAME(const VertexBase&, int index) const = 0;				\
};																							\
																							\
template <int... Indices>																	\
class VertexPartReaderImpl<SEMANTIC, Indices...> : public VertexPartReader<SEMANTIC> {		\
	intptr_t offset;																		\
	static const std::vector<int> indices;													\
	inline VertexPart<SEMANTIC, Indices...>& Cast(VertexBase& obj) const {					\
		return *(VertexPart<SEMANTIC, Indices...>*)((intptr_t)&obj + offset);				\
	}																						\
	inline const VertexPart<SEMANTIC, Indices...>& Cast(const VertexBase& obj) const {		\
		return *(const VertexPart<SEMANTIC, Indices...>*)((intptr_t)&obj + offset);			\
	}																						\
public:																						\
	VertexPartReaderImpl(intptr_t offset) : offset(offset) {}								\
	DataType& GET_NAME(VertexBase& obj, int index) const override { return Cast(obj).MULTI_NAME[index]; }					\
	const DataType& GET_NAME(const VertexBase& obj, int index) const override { return Cast(obj).MULTI_NAME[index]; }		\
	const std::vector<int>& GET_NAME##Indices() const { return indices; }					\
	virtual std::type_index GetType() const override { return typeid(DataType); }			\
	virtual const std::vector<int>& GetIndices() const override { return indices; }			\
	virtual size_t GetSize() const override { return sizeof(DataType); }					\
	virtual void* GetPointer(VertexBase& obj, int index) const override {					\
		return & GET_NAME (obj, index);														\
	}																						\
	virtual const void* GetPointer(const VertexBase& obj, int index) const override {		\
		return &GET_NAME(obj, index);														\
	}																						\
};																							\
template <int... Indices>																	\
const std::vector<int> VertexPartReaderImpl<SEMANTIC, Indices...>::indices = { Indices... };




// Actual definition of vertex parts
INL_GXENG_VERTEX_PART(eVertexElementSemantic::POSITION, INL_GXENG_SIMPLE_ARG(Vec3_Packed), GetPosition, positions, position)
INL_GXENG_VERTEX_PART(eVertexElementSemantic::POSITION2D, INL_GXENG_SIMPLE_ARG(Vec2_Packed), GetPosition2D, position2ds, position2d)
INL_GXENG_VERTEX_PART(eVertexElementSemantic::POSITION4D, INL_GXENG_SIMPLE_ARG(Vec4_Packed), GetPosition4D, position4ds, position4d)
INL_GXENG_VERTEX_PART(eVertexElementSemantic::NORMAL, INL_GXENG_SIMPLE_ARG(Vec3_Packed), GetNormal, normals, normal)
INL_GXENG_VERTEX_PART(eVertexElementSemantic::TEX_COORD, INL_GXENG_SIMPLE_ARG(Vec2_Packed), GetTexCoord, texCoords, texCoord)
INL_GXENG_VERTEX_PART(eVertexElementSemantic::COLOR, INL_GXENG_SIMPLE_ARG(Vec3_Packed), GetColor, colors, color)
INL_GXENG_VERTEX_PART(eVertexElementSemantic::TANGENT, INL_GXENG_SIMPLE_ARG(Vec3_Packed), GetTangent, tangents, tangent)
INL_GXENG_VERTEX_PART(eVertexElementSemantic::BITANGENT, INL_GXENG_SIMPLE_ARG(Vec3_Packed), GetBitangent, bitangents, bitangent)




//------------------------------------------------------------------------------
namespace impl {


// get a list of unique semantics from a big list of semantics (== remove duplicates)
template <class Accumulator, class InputList>
class UniqueSemanticAccumulator;

template <eVertexElementSemantic... AccSems, eVertexElementSemantic InSem1, eVertexElementSemantic... InSems>
class UniqueSemanticAccumulator<SemanticList<AccSems...>, SemanticList<InSem1, InSems...>> {
	using AccType = SemanticList<AccSems...>;
public:
	using type = typename std::conditional<ContainsSemantic<InSem1, AccType>::value,
		typename UniqueSemanticAccumulator<AccType, SemanticList<InSems...>>::type,
		typename UniqueSemanticAccumulator<typename impl::ConcatSemanticList<AccType, SemanticList<InSem1>>::type, SemanticList<InSems...>>::type>::type;
};

template <eVertexElementSemantic... AccSems>
class UniqueSemanticAccumulator<SemanticList<AccSems...>, SemanticList<>> {
public:
	using type = SemanticList<AccSems...>;
};
	
template <eVertexElementSemantic... Semantics>
class UniqueSemantics {
public:
	using type = typename UniqueSemanticAccumulator<SemanticList<>, SemanticList<Semantics...>>::type;
	static const std::vector<eVertexElementSemantic> list;
};

template <eVertexElementSemantic... Semantics>
const std::vector<eVertexElementSemantic> UniqueSemantics<Semantics...>::list = { Semantics... };


// get a list of vertex elements with given semantic from a big list of elements
template <eVertexElementSemantic Semantic, class... Elements>
class FilterElements;

template <eVertexElementSemantic Semantic, class Element1, class... Elements>
class FilterElements<Semantic, Element1, Elements...> {
public:
	using type = typename std::conditional<Element1::semantic == Semantic,
		typename ConcatTypeList<TypeList<Element1>, typename FilterElements<Semantic, Elements...>::type>::type,
		typename FilterElements<Semantic, Elements...>::type>::type;

};

template <eVertexElementSemantic Semantic>
class FilterElements<Semantic> {
public:
	using type = TypeList<>;
};


// inherit from a vertex part corresponding to multiple elements (of the same semantic)
template <class... FilteredElements>
class VertexPartRealization;

template <eVertexElementSemantic Semantic1, int Index1>
class VertexPartRealization<VertexElement<Semantic1, Index1>>
	: public VertexPart<Semantic1, Index1>
{};

template <eVertexElementSemantic Semantic1, int Index1, int... Indices>
class VertexPartRealization<VertexElement<Semantic1, Index1>, VertexElement<Semantic1, Indices>...>
	: public VertexPart<Semantic1, Index1, Indices...>
{};


// helper class to translate typelist to elements...
template <class T>
class VertexPartRealizationDemuxer;

template <class... Elements>
class VertexPartRealizationDemuxer<TypeList<Elements...>> : public VertexPartRealization<Elements...> {};


// go over all unique semantics, and inherit corresponding semantic's vertex part
template <class Uniques, class... Elements>
class VertexPartAccumulator;

template <eVertexElementSemantic FirstUnique, eVertexElementSemantic... Uniques, class... Elements>
class VertexPartAccumulator<SemanticList<FirstUnique, Uniques...>, Elements...>
	: public VertexPartRealizationDemuxer<typename FilterElements<FirstUnique, Elements...>::type>,
	public VertexPartAccumulator<SemanticList<Uniques...>, Elements...>
{};

template <class... Elements>
class VertexPartAccumulator<SemanticList<>, Elements...>
{};



// inherit from a vertex part READER corresponding to multiple elements (of the same semantic)
template <class VertexT, class... FilteredElements>
class VertexPartReaderRealization;

template <class VertexT, eVertexElementSemantic Semantic1, int Index1>
class VertexPartReaderRealization<VertexT, VertexElement<Semantic1, Index1>>
	: public VertexPartReaderImpl<Semantic1, Index1>
{
public:
	VertexPartReaderRealization() 
		: VertexPartReaderImpl<Semantic1, Index1>((intptr_t)static_cast<VertexPart<Semantic1, Index1>*>((VertexT*)1000) - 1000) {}
protected:
	const VertexPartReaderBase* GetPartReaderBase() const {
		return static_cast<const VertexPartReader<Semantic1>*>(this);
	}
};

template <class VertexT, eVertexElementSemantic Semantic1, int Index1, int... Indices>
class VertexPartReaderRealization<VertexT, VertexElement<Semantic1, Index1>, VertexElement<Semantic1, Indices>...>
	: public VertexPartReaderImpl<Semantic1, Index1, Indices...>
{
public:
	VertexPartReaderRealization() 
		: VertexPartReaderImpl<Semantic1, Index1, Indices...>((intptr_t)static_cast<VertexPart<Semantic1, Index1, Indices...>*>((VertexT*)1000) - 1000) {}
protected:
	const VertexPartReaderBase* GetPartReaderBase() const {
		return static_cast<const VertexPartReader<Semantic1>*>(this);
	}
};


template <class VertexT, class T>
class VertexPartReaderRealizationDemuxer;

template <class VertexT, class... Elements>
class VertexPartReaderRealizationDemuxer<VertexT, TypeList<Elements...>> : public VertexPartReaderRealization<VertexT, Elements...> {
protected:
	using VertexPartReaderRealization<VertexT, Elements...>::GetPartReaderBase;
};


template <class VertexT, class Uniques, class... Elements>
class VertexPartReaderAccumulator;

template <class VertexT, eVertexElementSemantic FirstUnique, eVertexElementSemantic... Uniques, class... Elements>
class VertexPartReaderAccumulator<VertexT, SemanticList<FirstUnique, Uniques...>, Elements...>
	: public VertexPartReaderRealizationDemuxer<VertexT, typename FilterElements<FirstUnique, Elements...>::type>,
	public VertexPartReaderAccumulator<VertexT, SemanticList<Uniques...>, Elements...>
{
protected:
	const VertexPartReaderBase* GetPartReaderBase(eVertexElementSemantic semantic) const {
		if (FirstUnique == semantic) {
			return VertexPartReaderRealizationDemuxer<VertexT, typename FilterElements<FirstUnique, Elements...>::type>::GetPartReaderBase();
		}
		else {
			return VertexPartReaderAccumulator<VertexT, SemanticList<Uniques...>, Elements...>::GetPartReaderBase(semantic);
		}
	}
};

template <class VertexT, class... Elements>
class VertexPartReaderAccumulator<VertexT, SemanticList<>, Elements...>
{
protected:
	const VertexPartReaderBase* GetPartReaderBase(eVertexElementSemantic semantic) const {
		return nullptr;
	}
};




} // namespace impl
//------------------------------------------------------------------------------



/// <summary>
/// Passed to the Mesh so that it can understand the vertex content.
/// </summary>
/// <remarks>
/// Automatically generated for templated vertices, and can be accessed through
/// Vertex::GetReader.
/// For custom vertices, implement this interface, but keep in mind that the underlying
/// type of vertex parts cannot be changed.
/// </remarks>
class IVertexReader {
public:
	struct Element {
		eVertexElementSemantic semantic;
		int index;
	};

	template <eVertexElementSemantic Semantic>
	const VertexPartReader<Semantic>* GetPartReader() const {
		return dynamic_cast<const VertexPartReader<Semantic>*>(GetPartReaderBase(Semantic));
	}

	virtual const std::vector<eVertexElementSemantic>& GetSemantics() const = 0;
	virtual int GetStride() const = 0;

	const std::vector<int>& GetIndices(eVertexElementSemantic semantic) const {
		const VertexPartReaderBase* reader = GetPartReaderBase(semantic);
		assert(reader != nullptr);
		return reader->GetIndices();
	}

	void* GetPointer(VertexBase& vertex, eVertexElementSemantic semantic, int index) const {
		const VertexPartReaderBase* reader = GetPartReaderBase(semantic);
		assert(reader != nullptr);
		return reader->GetPointer(vertex, index);
	}

	const void* GetPointer(const VertexBase& vertex, eVertexElementSemantic semantic, int index) const {
		const VertexPartReaderBase* reader = GetPartReaderBase(semantic);
		assert(reader != nullptr);
		return reader->GetPointer(vertex, index);
	}

	std::type_index GetType(eVertexElementSemantic semantic) const {
		const VertexPartReaderBase* reader = GetPartReaderBase(semantic);
		assert(reader != nullptr);
		return reader->GetType();
	}

	std::size_t GetSize(eVertexElementSemantic semantic) const {
		const VertexPartReaderBase* reader = GetPartReaderBase(semantic);
		assert(reader != nullptr);
		return reader->GetSize();
	}

	virtual const std::vector<Element>& GetElements() const = 0;
protected:
	virtual const VertexPartReaderBase* GetPartReaderBase(eVertexElementSemantic semantic) const = 0;
};


template <class... Elements>
class VertexReader;


class VertexBase
{};

template <class... Elements>
class Vertex;



template <eVertexElementSemantic... Semantics, int... Indices>
class VertexReader<VertexElement<Semantics, Indices>...>
	: public IVertexReader,
	public impl::VertexPartReaderAccumulator<Vertex<VertexElement<Semantics, Indices>...>, typename impl::UniqueSemantics<Semantics...>::type, VertexElement<Semantics, Indices>...>
{
public:
	const std::vector<eVertexElementSemantic>& GetSemantics() const override {
		return semantics;
	}
	int GetStride() const override {
		return (int)sizeof(Vertex<VertexElement<Semantics, Indices>...>);
	}
	const std::vector<Element>& GetElements() const override {
		return elements;
	}
protected:
	const VertexPartReaderBase* GetPartReaderBase(eVertexElementSemantic semantic) const override {
		return impl::VertexPartReaderAccumulator<Vertex<VertexElement<Semantics, Indices>...>, typename impl::UniqueSemantics<Semantics...>::type, VertexElement<Semantics, Indices>...>::GetPartReaderBase(semantic);
	}
private:
	static const std::vector<eVertexElementSemantic> semantics;
	static const std::vector<Element> elements;
};

template <eVertexElementSemantic... Semantics, int... Indices>
const std::vector<eVertexElementSemantic> VertexReader<VertexElement<Semantics, Indices>...>::semantics = impl::UniqueSemantics<Semantics...>::list;

template <eVertexElementSemantic... Semantics, int... Indices>
const std::vector<IVertexReader::Element> VertexReader<VertexElement<Semantics, Indices>...>::elements = {
	{ Semantics, Indices }...
};


/// <summary>
/// Procedural vertex class. Specify the vertex structure's contents by the template parameters.
/// The specified contents are accessible as member variables.
/// Pass an array of vertices along with corresponding reader to the <see cref="Mesh"/> class.
/// </summary>
template <eVertexElementSemantic... Semantics, int... Indices>
class Vertex<VertexElement<Semantics, Indices>...> 
	: public VertexBase,
	public impl::VertexPartAccumulator<typename impl::UniqueSemantics<Semantics...>::type, VertexElement<Semantics, Indices>...>
{
public:
	static VertexReader<VertexElement<Semantics, Indices>...>& GetReader() {
		static VertexReader<VertexElement<Semantics, Indices>...> reader;
		return reader;
	}
};




} // namespace inl::gxeng