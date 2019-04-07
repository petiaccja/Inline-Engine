#pragma once


#include "ComponentScheme.hpp"
#include "ContiguousVector.hpp"

#include "BaseLibrary/Exception/Exception.hpp"
#include "BaseLibrary/Range.hpp"

#include <any>
#include <array>


namespace inl::game {


namespace impl {
	template <class T, size_t N>
	std::array<T, N> InvertPermutation(const std::array<T, N>& perm) {
		std::array<T, N> iperm;
		for (size_t i = 0; i < N; ++i) {
			iperm[perm[i]] = i;
		}
		return iperm;
	}
} // namespace impl


/// <summary> An absolutely not typesafe wrapper for contiguous vectors to store any type.
///		To be used ONLY for the <see cref="EntityStore"/>. </summary>
class IComponentVector {
public:
	~IComponentVector() = default;

	/// <summary> Returns the number of items in the vector. </summary>
	virtual size_t Size() const = 0;

	/// <summary> Copies a contiguous array of objects into the end of the container. </summary>
	/// <param name="firstObject"> Pointer to the first object. No type checking is done, watch out. </param>
	/// <param name="count"> Number of objects to copy. </param>
	virtual void PushBack(const void* firstObject, size_t count = 1) = 0;

	/// <summary> Moves a contiguous array of objects into the end of the container. </summary>
	/// <param name="firstObject"> Pointer to the first object. No type checking is done, watch out. </param>
	/// <param name="count"> Number of objects to move. </param>
	virtual void PushBack(void* firstObject, size_t count = 1) = 0;

	/// <summary> Erases the specified element from the contiguous vector. </summary>
	virtual void Erase(size_t index) = 0;

	/// <summary> Moves an element from <paramref name="other"/> to the end of *this. </summary>
	/// <param name="other"> The container to remove the element from. </param>
	/// <param name="index"> The index of the element to remove from <paramref name="other"/>. </param>
	virtual void SpliceBack(IComponentVector& other, size_t index) = 0;

	/// <summary> Return the type of the stored objects. </summary>
	virtual std::type_index GetType() = 0;

	/// <summary> Creates a component vector of the same type but empty. </summary>
	virtual std::unique_ptr<IComponentVector> Clone() const = 0;
};



template <class ComponentType>
class ComponentVector final : public IComponentVector {
public:
	ComponentVector() = default;
	ComponentVector(std::initializer_list<ComponentType> list);
	size_t Size() const override;
	void PushBack(const void*, size_t count = 1) override;
	void PushBack(void*, size_t count = 1) override;
	void Erase(size_t index) override;
	void SpliceBack(IComponentVector& other, size_t index) override;
	void Resize(size_t size);
	std::type_index GetType() override;
	const ContiguousVector<ComponentType>& Container() const;
	std::unique_ptr<IComponentVector> Clone() const override;

private:
	ContiguousVector<ComponentType> m_container;
};



class EntityStore {
public:
	/// <summary> Insert a new entity made up from specified components. </summary>
	/// <exception cref="inl::InvalidArgumentException"> If the provided components do not match the scheme of *this exactly. </exception>
	template <class... Components>
	void PushBack(Components&&... components);

	/// <summary> Moves the <paramref name="index"/>th container from <paramref name="other"/> into *this. </summary>
	/// <param name="other"> The store to remove the element from. </param>
	/// <param name="index"> Which element to remove. </param>
	/// <param name="extraComponents"> In case *this's scheme is a superset of other's, you can provide the extra components. </param>
	/// <remarks> The scehem of *this must exactly match the scheme of <paramref name="other"/>
	///		union <paramref name="extraComponents"/>. </remarks>
	template <class... Components>
	void SpliceBackExtend(EntityStore& other, size_t index, Components&&... extraComponents);

	/// <summary> Moves the <paramref name="index"/>th container from <paramref name="other"/> into *this. </summary>
	/// <param name="other"> The store to remove the element from. </param>
	/// <param name="index"> Which element set to remove. </param>
	/// <param name="selection"> Which subset of components to keep. </param>
	/// <remarks> The scheme of *this must be a subset of the scheme of <paramref name="other"/>. </remarks>
	void SpliceBackReduce(EntityStore& other, size_t index, const std::vector<bool>& selection);

	/// <summary> Erases the <paramref name="index"/>th component from each component vector.</summary>
	void Erase(size_t index);

	/// <summary> Adds a new component type vector to the store, default initialized. </summary>
	template <class ComponentType, class... ExtraTypes>
	void Extend();

	/// <summary> Adds a new component type vector to the store, initialized with <paramref name="data"/>. </summary>
	template <class ComponentType>
	void Extend(std::initializer_list<ComponentType>& data);

	/// <summary> Removed the first occurence of the component type specified. </summary>
	template <class ComponentType>
	void Reduce();

	/// <summary> Remove the component type specified by index. </summary>
	void Reduce(size_t index);

	/// <summary> Return the number of components for one component type (same number for all component types). </summary>
	size_t Size() const;

	/// <summary> Returns the scheme for the set of component types the store contains. </summary>
	const ComponentScheme& Scheme() const;

	/// <summary> Creates an EntityStore with the exact same component vectors as this, but with all vectors empty. </summary>
	EntityStore CloneScheme() const;

	/// <summary> Return the <paramref name="index"/>th vector of components. </summary>
	/// <exception cref="InvalidCastException"> In case you have given the wrong type. </exception>
	/// <remarks> You must specify the type correctly. </remarks>
	template <class ComponentT>
	const ContiguousVector<ComponentT>& GetComponentVector(size_t index) const;


private:
	template <size_t Index, class... ComponentT>
	void PushBackHelper(const std::tuple<ComponentT...>& components, const std::array<size_t, sizeof...(ComponentT)>& reorder);

private:
	std::vector<std::unique_ptr<IComponentVector>> m_components;
	ComponentScheme m_scheme;
};



template <class ComponentType>
ComponentVector<ComponentType>::ComponentVector(std::initializer_list<ComponentType> list) : m_container{ list } {}


template <class ComponentType>
void ComponentVector<ComponentType>::Resize(size_t size) {
	m_container.resize(size);
}


template <class ComponentType>
std::type_index ComponentVector<ComponentType>::GetType() {
	return typeid(ComponentType);
}


template <class ComponentType>
const ContiguousVector<ComponentType>& ComponentVector<ComponentType>::Container() const {
	return m_container;
}


template <class ComponentType>
std::unique_ptr<IComponentVector> ComponentVector<ComponentType>::Clone() const {
	return std::make_unique<ComponentVector>();
}


template <class... Components>
void EntityStore::PushBack(Components&&... components) {
	static const ComponentScheme scheme{ std::type_index(typeid(Components))... };
	static const std::array<size_t, sizeof...(Components)> reorder = [] {
		std::array<size_t, sizeof...(Components)> reorder;
		std::array<std::type_index, sizeof...(Components)> types = { typeid(Components)... };
		for (auto i : Range(reorder.size()))
			reorder[i] = i;
		std::stable_sort(reorder.begin(), reorder.end(), [&types](size_t lhs, size_t rhs) {
			return types[lhs] < types[rhs];
		});
		return impl::InvertPermutation(reorder);
	}();

	if (scheme == m_scheme) {
		PushBackHelper<0>(std::forward_as_tuple(std::forward<Components>(components)...), reorder);
	}
	else {
		throw InvalidArgumentException();
	}
}


template <class... Components>
void EntityStore::SpliceBackExtend(EntityStore& other, size_t index, Components&&... extraComponents) {
	static const auto reorder = [] {
		std::array<size_t, sizeof...(Components)> reorder;
		std::array<std::type_index, sizeof...(Components)> types = { typeid(Components)... };
		for (auto i : Range(reorder.size()))
			reorder[i] = i;
		std::stable_sort(reorder.begin(), reorder.end(), [&types](size_t lhs, size_t rhs) {
			return types[lhs] < types[rhs];
		});
		return reorder;
	}();

	static const auto types = std::array{ std::type_index(typeid(Components))... };
	static const auto constness = std::array{ std::is_rvalue_reference_v<Components>... };
	static const auto extraComponentPtrs = std::array{ static_cast<const void*>(&extraComponents)... };

	size_t myVectorIndex = 0;
	size_t otherVectorIndex = 0;
	size_t extraIndex = 0;
	size_t otherVectorSize = other.m_components.size();
	size_t extraSize = sizeof...(Components);

	while (myVectorIndex < m_components.size()) {
		// Either must be true due to loop condition.
		bool otherHasMore = otherVectorIndex < otherVectorSize;
		bool extraHasMore = extraIndex < extraSize;
		auto& myVector = *m_components[myVectorIndex];

		// Splice components from other.
		if (!extraHasMore || (otherHasMore && other.m_components[otherVectorIndex]->GetType() <= types[reorder[extraIndex]])) {
			myVector.SpliceBack(*other.m_components[otherVectorIndex], index);
			++otherVectorIndex;
		}
		// Push back component from extras.
		else {
			size_t reorderedIndex = reorder[extraIndex];
			assert(types[reorderedIndex] == m_components[myVectorIndex]->GetType());
			if (constness[reorderedIndex]) {
				myVector.PushBack(extraComponentPtrs[reorderedIndex]);
			}
			else {
				myVector.PushBack(const_cast<void*>(extraComponentPtrs[reorderedIndex]));
			}
			++extraIndex;
		}
		++myVectorIndex;
	}
}


template <class ComponentType>
size_t ComponentVector<ComponentType>::Size() const {
	return m_container.size();
}


template <class ComponentType>
void ComponentVector<ComponentType>::PushBack(const void* firstObject, size_t count) {
	auto firstComponent = reinterpret_cast<const ComponentType*>(firstObject);
	auto lastComponent = firstComponent + count;
	for (; firstComponent != lastComponent; ++firstComponent) {
		m_container.push_back(*firstComponent);
	}
}


template <class ComponentType>
void ComponentVector<ComponentType>::PushBack(void* firstObject, size_t count) {
	auto firstComponent = reinterpret_cast<ComponentType*>(firstObject);
	auto lastComponent = firstComponent + count;
	for (; firstComponent != lastComponent; ++firstComponent) {
		m_container.push_back(std::move(*firstComponent));
	}
}


template <class ComponentType>
void ComponentVector<ComponentType>::Erase(size_t index) {
	m_container.erase(m_container.begin() + index);
}


template <class ComponentType>
void ComponentVector<ComponentType>::SpliceBack(IComponentVector& other, size_t index) {
	assert(GetType() == other.GetType()); // Check in debug mode via assert should be enough.
	auto& otherTyped = static_cast<ComponentVector<ComponentType>&>(other);
	auto movedIt = otherTyped.m_container.begin() + index;
	m_container.push_back(std::move(*movedIt));
	otherTyped.m_container.erase(movedIt);
}


template <class ComponentType, class... ExtraTypes>
void EntityStore::Extend() {
	ComponentVector<ComponentType> vec;
	const auto schemeIt = m_scheme.Insert(typeid(ComponentType));
	const auto schemeIndex = schemeIt - m_scheme.begin();
	m_components.insert(m_components.begin() + schemeIndex, std::make_unique<decltype(vec)>(std::move(vec)));

	if constexpr (sizeof...(ExtraTypes) > 0) {
		Extend<ExtraTypes...>();
	}
}


template <class ComponentType>
void EntityStore::Extend(std::initializer_list<ComponentType>& data) {
	ComponentVector<ComponentType> vec{ data };
	vec.Resize(Size());

	const auto schemeIt = m_scheme.Insert(typeid(ComponentType));
	const auto schemeIndex = schemeIt - m_scheme.begin();
	m_components.insert(m_components.begin() + schemeIndex, std::make_unique<decltype(vec)>(std::move(vec)));
}


template <class ComponentType>
void EntityStore::Reduce() {
	auto [firstIt, lastIt] = m_scheme.Range(typeid(ComponentType));

	if (firstIt == lastIt) {
		throw InvalidArgumentException("Given type if not member of this scheme.");
	}

	const size_t index = std::distance(m_scheme.begin(), firstIt);
	m_scheme.Erase(firstIt);
	m_components.erase(m_components.begin() + index);
}


template <class ComponentT>
const ContiguousVector<ComponentT>& EntityStore::GetComponentVector(size_t index) const {
	const auto* cv = dynamic_cast<const ComponentVector<ComponentT>*>(m_components[index].get());
	if (cv) {
		return cv->Container();
	}
	throw InvalidCastException("Be sure to specify the correct component type.");
}


template <size_t Index, class... ComponentT>
void EntityStore::PushBackHelper(const std::tuple<ComponentT...>& components, const std::array<size_t, sizeof...(ComponentT)>& reorder) {
	if constexpr (Index < sizeof...(ComponentT)) {
		auto& currentVector = *m_components[reorder[Index]];
		using CurrentType = std::tuple_element_t<Index, std::decay_t<decltype(components)>>;

		assert(currentVector.GetType() == typeid(CurrentType)); // May need the inverse permutation (reorder)?

		constexpr bool shouldMove = std::is_rvalue_reference_v<CurrentType>;
		using PointerType = std::conditional_t<shouldMove, void*, const void*>;

		currentVector.PushBack(reinterpret_cast<PointerType>(&std::get<Index>(components)));

		PushBackHelper<Index + 1>(components, reorder);
	}
}



} // namespace inl::game
