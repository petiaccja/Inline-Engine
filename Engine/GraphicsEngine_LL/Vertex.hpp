#pragma once

#include <type_traits>
#include <stdexcept>


enum eVertexElementSemantic {
    POSITION,
    NORMAL,
    COLOR,
};


template <eVertexElementSemantic Semantic, int Index>
class VertexElement {
public:
    static constexpr eVertexElementSemantic semantic = Semantic;
    static constexpr int index = Index;
};


template <eVertexElementSemantic Semantic, int... Indices>
class SemanticContainer;


template <eVertexElementSemantic Semantic>
class SemanticContainer<Semantic> {};



template <class T, int... Indices>
class SemanticContainerIndexer {
private:
    static constexpr int count = sizeof...(Indices);
    static constexpr int table[count] = {Indices...};
protected:
    T values[count];
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

template <class T, int... Indices>
constexpr int SemanticContainerIndexer<T, Indices...>::table[count];


template <eVertexElementSemantic Semantic, int... Indices>
class SemanticMultiContainer;









template <eVertexElementSemantic Semantic, int Index1, int Index2, int... Indices>
class SemanticContainer<Semantic, Index1, Index2, Indices...> : public SemanticMultiContainer<Semantic, Index1, Index2, Indices...> {

};


template <eVertexElementSemantic... Semantics>
class SemanticList {};

template <class T, class U>
class ConcatSemanticList;

template <eVertexElementSemantic... Semantics1, eVertexElementSemantic... Semantics2>
class ConcatSemanticList<SemanticList<Semantics1...>, SemanticList<Semantics2...>> {
public:
    using type = SemanticList<Semantics1..., Semantics2...>;
};


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



class EmptyClass {};


template <class... Types>
class TypeList {};

template <class T, class U>
class ConcatTypeList;

template <class... Types1, class... Types2>
class ConcatTypeList<TypeList<Types1...>, TypeList<Types2...>> {
public:
    using type = TypeList<Types1..., Types2...>;
};


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
    ElementFilterHelper() {
        cout << Semantic << ":";
        const int array[] = {Elements::index...};
        for (auto v : array) {
            cout << " " << v;
        }
        cout << endl;
    }
};

template <eVertexElementSemantic Semantic, class... Elements>
class ElementFilter : public ElementFilterHelper<TypeList<>, Semantic, Elements...> {
public:
};

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

template <class... Elements>
class Vertex : public VertexHelper<SemanticList<>, Elements...>
{};




template <int... Indices>
class SemanticMultiContainer<POSITION, Indices...> {
public:
    SemanticContainerIndexer<float, Indices...> positions;
};

template <int SingleIndex>
class SemanticContainer<POSITION, SingleIndex> : public SemanticMultiContainer<POSITION, SingleIndex> {
public:
    SemanticContainer() : position(this->positions[SingleIndex]) {}
    float& position;
};

template <int... Indices>
class SemanticMultiContainer<NORMAL, Indices...> {
public:
    SemanticContainerIndexer<float, Indices...> normals;
};

template <int SingleIndex>
class SemanticContainer<NORMAL, SingleIndex> : public SemanticMultiContainer<NORMAL, SingleIndex> {
public:
    SemanticContainer() : normal(this->normals[SingleIndex]) {}
    float &normal;
};

template <int... Indices>
class SemanticMultiContainer<COLOR, Indices...> {
public:
    SemanticContainerIndexer<float, Indices...> colors;
};

template <int SingleIndex>
class SemanticContainer<COLOR, SingleIndex> : public SemanticMultiContainer<COLOR, SingleIndex> {
public:
    SemanticContainer() : color(this->colors[SingleIndex]) {}
    float &color;
};


template <int Index>
using Position = VertexElement<POSITION, Index>;

template <int Index>
using Normal = VertexElement<NORMAL, Index>;

template <int Index>
using Color = VertexElement<NORMAL, Index>;

