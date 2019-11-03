#include "ContiguousVector.hpp"

#include <BaseLibrary/Exception/Exception.hpp>

#include <typeindex>

namespace inl::game {

class ComponentVectorBase {
public:
	template <class Component>
	void PushBack(Component&& component);

	template <class Component>
	void Insert(size_t where, Component&& component);

	template <class Component>
	void PushBack(const Component& component);

	template <class Component>
	void Insert(size_t where, const Component& component);

	virtual void PushBackDefault() = 0;
	virtual void InsertDefault(size_t where) = 0;
	virtual void Resize(size_t size) = 0;
	virtual void Erase(size_t where) = 0;
	virtual void Erase(size_t first, size_t last) = 0;

	virtual size_t Size() const = 0;
	virtual std::type_index Type() const = 0;

	virtual void Copy(size_t targetIndex, const ComponentVectorBase& sourceVector, size_t sourceIndex) = 0;
	virtual void Move(size_t targetIndex, const ComponentVectorBase& sourceVector, size_t sourceIndex) = 0;

protected:
	virtual void InsertMove(size_t where, void* componentPtr) = 0;
	virtual void InsertCopy(size_t where, const void* componentPtr) = 0;

private:
	template <class Component>
	bool CheckType() const;
};


template <class T>
class _ComponentVector : public ComponentVectorBase {
public:
	void PushBackDefault() override;
	void InsertDefault(size_t where) override;
	void Resize(size_t size) override;
	void Erase(size_t where) override;
	void Erase(size_t first, size_t last) override;
	size_t Size() const override;
	std::type_index Type() const override;
	T& operator[](size_t index);
	const T& operator[](size_t index) const;
	void Copy(size_t targetIndex, const ComponentVectorBase& sourceVector, size_t sourceIndex) override;
	void Move(size_t targetIndex, const ComponentVectorBase& sourceVector, size_t sourceIndex) override;

protected:
	void InsertMove(size_t where, void* componentPtr) override;
	void InsertCopy(size_t where, const void* componentPtr) override;

private:
	ContiguousVector<T> m_data;
};



template <class Component>
void ComponentVectorBase::PushBack(Component&& component) {
	Insert(Size(), std::move(component));
}

template <class Component>
void ComponentVectorBase::Insert(size_t where, Component&& component) {
	if (!CheckType<Component>()) {
		throw InvalidArgumentException("Argument type mismatch.");
	}
	InsertMove(where, &component);
}

template <class Component>
void ComponentVectorBase::PushBack(const Component& component) {
	Insert(Size(), component);
}

template <class Component>
void ComponentVectorBase::Insert(size_t where, const Component& component) {
	if (!CheckType<Component>()) {
		throw InvalidArgumentException("Argument type mismatch.");
	}
	InsertCopy(where, &component);
}

template <class Component>
bool ComponentVectorBase::CheckType() const {
	return typeid(Component) == Type();
}

template <class T>
void _ComponentVector<T>::PushBackDefault() {
	m_data.push_back({});
}

template <class T>
void _ComponentVector<T>::InsertDefault(size_t where) {
	m_data.insert(m_data.begin() + where, T{});
}

template <class T>
void _ComponentVector<T>::Resize(size_t size) {
	m_data.resize(size);
}

template <class T>
void _ComponentVector<T>::Erase(size_t where) {
	m_data.erase(m_data.begin() + where);
}

template <class T>
void _ComponentVector<T>::Erase(size_t first, size_t last) {
	m_data.erase(m_data.begin() + first, m_data.begin() + last);
}

template <class T>
size_t _ComponentVector<T>::Size() const {
	return m_data.size();
}

template <class T>
std::type_index _ComponentVector<T>::Type() const {
	return typeid(T);
}

template <class T>
void _ComponentVector<T>::InsertMove(size_t where, void* componentPtr) {
	m_data.insert(m_data.begin() + where, std::move(*reinterpret_cast<T*>(componentPtr)));
}

template <class T>
void _ComponentVector<T>::InsertCopy(size_t where, const void* componentPtr) {
	m_data.insert(m_data.begin() + where, *reinterpret_cast<const T*>(componentPtr));
}

template <class T>
T& _ComponentVector<T>::operator[](size_t index) {
	return m_data[index];
}

template <class T>
const T& _ComponentVector<T>::operator[](size_t index) const {
	return m_data[index];
}

template <class T>
void _ComponentVector<T>::Copy(size_t targetIndex, const ComponentVectorBase& sourceVector, size_t sourceIndex) {
	auto& sourceVectorTyped = dynamic_cast<const _ComponentVector<T>&>(sourceVector);
	(*this)[targetIndex] = sourceVectorTyped[sourceIndex];
}

template <class T>
void _ComponentVector<T>::Move(size_t targetIndex, const ComponentVectorBase& sourceVector, size_t sourceIndex) {
	auto& sourceVectorTyped = dynamic_cast<const _ComponentVector<T>&>(sourceVector);
	(*this)[targetIndex] = std::move(sourceVectorTyped[sourceIndex]);
}


} // namespace inl::game