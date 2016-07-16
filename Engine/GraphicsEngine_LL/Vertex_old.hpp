#pragma once

#include <type_traits>
#include <stdexcept>

#include <mathfu/vector.h>


// Available semantics.
enum eVertexElementSemantic {
    POSITION,
    NORMAL,
};


/// <summary> Describes a vertex element, which has 2 properties: semantic and index. </summary>
template <eVertexElementSemantic Semantic, int Index>
class VertexElement {
public:
    static constexpr eVertexElementSemantic semantic = Semantic;
    static constexpr int index = Index;
};


// Stores values related to a certain semantic. The type of the values and list of indices is specified.
template <class T, int... Indices>
class SemanticContainerIndexer {
private:
    static constexpr int count = sizeof...(Indices);
    static constexpr int table[count] = {Indices...};
protected:
    T values[count]; //
public:
    T& operator[](int index) {
        for (int lookupIdx = 0; lookupIdx < count; ++lookupIdx) {
            if (table[lookupIdx] == index) {
                return values[lookupIdx];
            }
        }
        throw std::invalid_argument("Index not found.");
    }
    const T& operator[](int index) const {
        for (int lookupIdx = 0; lookupIdx < count; ++lookupIdx) {
            if (table[lookupIdx] == index) {
                return values[lookupIdx];
            }
        }
        throw std::invalid_argument("Index not found.");
    }
};

// Just the definition of statics...
template <class T, int... Indices>
constexpr int SemanticContainerIndexer<T, Indices...>::table[count];




// Collect all indices that belong to a single semantic.
template <eVertexElementSemantic Semantic, int... Indices>
class SemanticContainer;


// Specialization for semantic with NO indices.
template <eVertexElementSemantic Semantic>
class SemanticContainer<Semantic>
{};


// Helper for semantic with MULTIPLE indices.
template <eVertexElementSemantic Semantic, int... Indices>
class SemanticMultiContainer;


// Specialization to implement MULTIPLE indices that inherits from the helper class.
template <eVertexElementSemantic Semantic, int Index1, int Index2, int... Indices>
class SemanticContainer<Semantic, Index1, Index2, Indices...> : public SemanticMultiContainer<Semantic, Index1, Index2, Indices...>
{};



// Template arguments form a list of semantics.
template <eVertexElementSemantic... Semantics>
class SemanticList {};

template <class T, class U>
class ConcatSemanticList;

template <eVertexElementSemantic... Semantics1, eVertexElementSemantic... Semantics2>
class ConcatSemanticList<SemanticList<Semantics1...>, SemanticList<Semantics2...>> {
public:
    using type = SemanticList<Semantics1..., Semantics2...>;
};


// Decides if a SemanticList contains a given Semantic.
template <eVertexElementSemantic Semantic, class List>
class ContainsSemantic;

template <eVertexElementSemantic Semantic, eVertexElementSemantic... List>
class ContainsSemanticHelper;

template <eVertexElementSemantic Semantic, eVertexElementSemantic Head, eVertexElementSemantic... List>
class ContainsSemanticHelper<Semantic, Head, List...> {
public:
    static constexpr bool value = (Semantic == Head) || ContainsSemanticHelper<Semantic, List...>::value;
};

template <eVertexElementSemantic Semantic>
class ContainsSemanticHelper<Semantic> {
public:
    static constexpr bool value = false;
};

template <eVertexElementSemantic Semantic, eVertexElementSemantic... List>
class ContainsSemantic<Semantic, SemanticList<List...>> {
public:
    static constexpr bool value = ContainsSemanticHelper<Semantic, List...>::value;
};


// Used to conditionally inherit from.
class EmptyClass {};


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


// Filter vertex elements that have the given semantic.
template <class Filtered, eVertexElementSemantic Semantic, class... Remaining>
class ElementFilterHelper;

template <class Filtered, eVertexElementSemantic Semantic, class Head, class... Remaining>
class ElementFilterHelper<Filtered, Semantic, Head, Remaining...>
        : public ElementFilterHelper<
                typename std::conditional<Head::semantic == Semantic, typename ConcatTypeList<Filtered, TypeList<Head>>::type, Filtered>::type,
                Semantic,
                Remaining...>
{};

template <class... Elements, eVertexElementSemantic Semantic>
class ElementFilterHelper<TypeList<Elements...>, Semantic>
    : public SemanticContainer<Semantic, Elements::index...>
{
public:
    //ElementFilterHelper() {
    //    cout << Semantic << ":";
    //    constexpr int array[] = {Elements::index...};
    //    for (auto v : array) {
    //        cout << " " << v;
    //    }
    //    cout << endl;
    //}
};

template <eVertexElementSemantic Semantic, class... Elements>
class ElementFilter : public ElementFilterHelper<TypeList<>, Semantic, Elements...>
{};


// Helper class to form a vertex.
// Groups vertex elements by semantic, then inherits from the container for that semantic.
template <class UsedSemantics, class... Elements>
class VertexHelper;

template <class UsedSemantics, eVertexElementSemantic FirstSemantic, eVertexElementSemantic... Semantics, int FirstIndex, int... Indices>
class VertexHelper<UsedSemantics, VertexElement<FirstSemantic, FirstIndex>, VertexElement<Semantics, Indices>...>
        : virtual public std::conditional<ContainsSemantic<FirstSemantic, UsedSemantics>::value,
                EmptyClass,
                ElementFilter<FirstSemantic, VertexElement<FirstSemantic, FirstIndex>, VertexElement<Semantics, Indices>...>>::type,
          public VertexHelper<typename ConcatSemanticList<UsedSemantics, SemanticList<FirstSemantic>>::type, VertexElement<Semantics, Indices>...>
{};

template <class UsedSemantics>
class VertexHelper<UsedSemantics>
{};



// Actual vertex class
template <class... Elements>
class Vertex : public VertexHelper<SemanticList<>, Elements...>
{};



// Definition of single- and multi-container classes PER SEMANTIC.
template <int... Indices>
class SemanticMultiContainer<POSITION, Indices...> {
public:
    SemanticContainerIndexer<mathfu::Vector<float, 3>, Indices...> positions;
};

template <int SingleIndex>
class SemanticContainer<POSITION, SingleIndex> : public SemanticMultiContainer<POSITION, SingleIndex> {
public:
    SemanticContainer() : position(this->positions[SingleIndex]) {}
	mathfu::Vector<float, 3>& position;
};


template <int... Indices>
class SemanticMultiContainer<NORMAL, Indices...> {
public:
    SemanticContainerIndexer<mathfu::Vector<float, 3>, Indices...> normals;
};

template <int SingleIndex>
class SemanticContainer<NORMAL, SingleIndex> : public SemanticMultiContainer<NORMAL, SingleIndex> {
public:
    SemanticContainer() : normal(this->normals[SingleIndex]) {}
	mathfu::Vector<float, 3> &normal;
};


// Type aliases for semantics
template <int Index>
using Position = VertexElement<POSITION, Index>;

template <int Index>
using Normal = VertexElement<NORMAL, Index>;
