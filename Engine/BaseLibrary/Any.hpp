#pragma once

#include <stdexcept>
#include <typeindex>
#include <memory>
#include "Exception/Exception.hpp"
using namespace inl;

namespace inl {


class Any {
	struct Data {
		virtual ~Data() = default;
		virtual void* Get() = 0;
		virtual const void* Get() const = 0;
		virtual size_t Size() const = 0;
		virtual Data* Clone() const = 0;
		virtual std::type_index Type() const = 0;
	};
public:
	Any() = default;

	template <class T>
	explicit Any(T&& obj) {
		struct DataSpec : public Data {
			DataSpec(const T& obj) : obj(obj) {}
			DataSpec(std::decay_t<T>&& obj) : obj(std::forward<T>(obj)) {}
			virtual void* Get() override { return &obj; };
			virtual const void* Get() const override { return &obj; }
			virtual size_t Size() const override { return sizeof(T); }
			virtual Data* Clone() const override { return new DataSpec(obj); }
			virtual std::type_index Type() const override { return typeid(T); }
			std::decay_t<T> obj;
		};
		m_data = std::make_unique<DataSpec>(std::forward<T>(obj));
	}

	Any(const Any& rhs) : m_data(rhs.m_data->Clone()) {}
	Any(Any&& rhs) : m_data(std::move(rhs.m_data)) {}
	Any& operator=(const Any& rhs) { m_data.reset(rhs.m_data->Clone()); return *this; }
	Any& operator=(Any&& rhs) { m_data = std::move(rhs.m_data); return *this; }

	template <class T>
	Any& operator=(T&& obj) {
		*this = Any(std::forward<T>(obj));
		return *this;
	}


	explicit operator bool() const { return (bool)m_data; }
	bool HasValue() const { return (bool)m_data; }


	template <class T>
	T& Get() {
		if (!m_data) {
			throw InvalidStateException("Object is empty.");
		}
		if (typeid(T) != m_data->Type()) {
			throw std::bad_cast("Types don't match.");
		}
		return *reinterpret_cast<T*>(m_data->Get());
	}

	template <class T>
	const T& Get() const {
		if (!m_data) {
			throw InvalidStateException("Object is empty.");
		}
		if (typeid(T) != m_data->Type()) {
			throw std::bad_cast("Types don't match.");
		}
		return *reinterpret_cast<const T*>(m_data->Get());
	}

	const void* Raw() const {
		if (!m_data) {
			throw InvalidStateException("Object is empty.");
		}
		return m_data->Get();
	}

	std::type_index Type() const {
		if (!m_data) {
			throw InvalidStateException("Object is empty.");
		}
		return m_data->Type();
	}
private:
	std::unique_ptr<Data> m_data;
};



} // namespace inl