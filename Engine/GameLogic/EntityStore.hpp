#pragma once


#include "ComponentScheme.hpp"
#include "ContiguousVector.hpp"

#include "BaseLibrary/Exception/Exception.hpp"
#include "BaseLibrary/Range.hpp"

#include <any>


namespace inl::game {


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
};


template <class ComponentType>
class ComponentVector final : IComponentVector {
public:
	ComponentVector() = default;
	ComponentVector(std::initializer_list<ComponentType> list);
	size_t Size() const override;
	void PushBack(const void*, size_t count = 1) override;
	void PushBack(void*, size_t count = 1) override;
	void Erase(size_t index) override;
	void SpliceBack(IComponentVector& other, size_t index) override;
	void Resize(size_t size);

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
	void SpliceBack(EntityStore& other, size_t index, Components&&... extraComponents);

	/// <summary> Moves the <paramref name="index"/>th container from <paramref name="other"/> into *this. </summary>
	/// <param name="other"> The store to remove the element from. </param>
	/// <param name="index"> Which element to remove. </param>
	/// <remarks> The scheme of *this must be a subset of the scheme of <paramref name="other"/>. </remarks>
	void SpliceBack(EntityStore& other, size_t index);


	void Erase(size_t index);

	template <class ComponentType>
	void Extend();

	template <class ComponentType>
	void Extend(std::initializer_list<ComponentType>& data);

	size_t Size() const;
	const ComponentScheme& Scheme() const;

private:
	std::vector<std::unique_ptr<IComponentVector>> m_components;
	ComponentScheme m_scheme;
};


template <class ComponentType>
void (*Insert)(std::any&, std::any) = [](std::any& cTarget, std::any cElems) {
	auto& t = std::any_cast<ContiguousVector<ComponentType>&>(cTarget);
	auto& s = std::any_cast<ContiguousVector<ComponentType>&>(cElems);
	for (auto& v : s) {
		t.push_back(std::move(v));
	}
};
template <class ComponentType>
void (*Erase)(std::any&, size_t) = [](std::any& cTarget, size_t index) {
	auto& t = std::any_cast<ContiguousVector<ComponentType>&>(cTarget);
	t.erase(t.begin() + index);
};
template <class ComponentType>
std::any (*Extract)(std::any&, size_t) = [](std::any& cTarget, size_t index) -> std::any {
	auto& t = std::any_cast<ContiguousVector<ComponentType>&>(cTarget);
	ContiguousVector<ComponentType> copy;
	copy.push_back(std::move(t[index]));
	t.erase(t.begin() + index);
	return std::any{ copy };
};


template <class ComponentType>
ComponentVector<ComponentType>::ComponentVector(std::initializer_list<ComponentType> list) : m_container{ list } {}


template <class ComponentType>
void ComponentVector<ComponentType>::Resize(size_t size) {
	m_container.resize(size);
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
	assert(typeid(other) == typeid(*this)); // Check in debug mode via assert should be enough.
	auto& otherTyped = static_cast<ComponentVector<ComponentType>&>(other);
	auto movedIt = otherTyped.m_container.begin() + index;
	m_container.push_back(std::move(*movedIt));
	otherTyped.m_container.erase(movedIt);
}


template <class ComponentType>
void EntityStore::Extend() {
	ComponentVector<ComponentType> vec;
	const auto schemeIt = m_scheme.Insert(typeid(ComponentType));
	const auto schemeIndex = schemeIt - m_scheme.begin();
	m_components.insert(m_components.begin() + schemeIndex, { typeid(ComponentType), std::move(vec) });
}


template <class ComponentType>
void EntityStore::Extend(std::initializer_list<ComponentType>& data) {
	ComponentVector<ComponentType> vec{ data };
	vec.Resize(Size());

	const auto schemeIt = m_scheme.Insert(typeid(ComponentType));
	const auto schemeIndex = schemeIt - m_scheme.begin();
	m_components.insert(m_components.begin() + schemeIndex, { typeid(ComponentType), std::move(vec) });
}


} // namespace inl::game
