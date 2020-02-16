#pragma once

#include "Delegate.hpp"
#include "SpinMutex.hpp"
#include "TemplateUtil.hpp"

#include <functional>
#include <mutex>
#include <set>
#include <typeindex>
#include <vector>


#ifdef _MSC_VER
#pragma warning(disable : 4180)
#endif


namespace inl {

namespace impl_tmp {
	// Remove this as soon as spaceship operator is properly added to the STL for type_index.
	inline std::strong_ordering operator<=>(const std::type_index& lhs, const std::type_index& rhs) {
		return lhs < rhs ? std::strong_ordering::less : (lhs == rhs ? std::strong_ordering::equal : std::strong_ordering::greater);
	}
} // namespace impl_tmp

/// <summary> You can sign up functors, then fire the event by calling it's (),
///		and all the signed up functors will be called. </summary>
template <class... ArgsT>
class Event {
	// Helper class to store comparable functors
	struct Comparable {
		template <class ComparableFun>
		Comparable(const ComparableFun& fun) {
			callee = fun;

			// Compares two comparable functors as -1, 0, +1 for less, equal and greater (like strcmp)
			compare = [](const std::function<void(ArgsT...)>& lhs, const std::function<void(ArgsT...)>& rhs) -> std::strong_ordering {
				const ComparableFun* target1 = lhs.target<const ComparableFun>();
				const ComparableFun* target2 = rhs.target<const ComparableFun>();
				if (target1 == nullptr || target2 == nullptr) {
					using impl_tmp::operator<=>;
					return std::type_index(lhs.target_type()) <=> std::type_index(rhs.target_type());
				}
				return *target1 < *target2 ? std::strong_ordering::less : (*target1 == *target2 ? std::strong_ordering::equal : std::strong_ordering::greater);
			};
		}

		// Calls the comparable functor.
		void operator()(ArgsT... args) const {
			callee(std::forward<ArgsT>(args)...);
		}
		// Check equality of underlying functors.
		bool operator==(const Comparable& rhs) const {
			return compare(callee, rhs.callee) == 0;
		}
		bool operator!=(const Comparable& rhs) const {
			return !(*this == rhs);
		}
		std::strong_ordering operator<=>(const Comparable& rhs) const {
			return compare(callee, rhs.callee);
		}

		std::function<void(ArgsT...)> callee; // Underlying functor.
		std::function<std::strong_ordering(const std::function<void(ArgsT...)>&, const std::function<void(ArgsT...)>&)> compare; // Comparison inner helper.
	};

	// Functors having both < and == are considered comparable.
	template <class Fun>
	static constexpr bool IsComparable = templ::is_equality_comparable<Fun>::value&& templ::is_less_comparable<Fun>::value;

public:
	Event() = default;

	/// <summary> All signed up functors are copied as well and signed up for the new event. </summary>
	Event(const Event& other) : m_simples(other.m_simples), m_comparables(other.m_comparables) {}

	/// <summary> All signed up functors are copied as well and signed up for the new event, old event is empty. </summary>
	Event(Event&& other) : m_simples(std::move(other.m_simples)), m_comparables(std::move(other.m_comparables)) {}

	/// <summary> All signed up functors are copied as well and signed up for the new event. </summary>
	Event& operator=(const Event& other) {
		std::lock_guard lkg(m_mtx);

		m_simples = other.m_simples;
		m_comparables = other.m_comparables;

		return *this;
	}

	/// <summary> All signed up functors are copied as well and signed up for the new event, old event is empty. </summary>
	Event& operator=(Event&& other) {
		std::lock_guard lkg(m_mtx);

		m_simples = std::move(other.m_simples);
		m_comparables = std::move(other.m_comparables);

		return *this;
	}

	/// <summary> Fire off the event, call all signed up functors with given arguments. </summary>
	void operator()(ArgsT... args) const {
		std::unique_lock<SpinMutex> lkg(m_mtx);
		// Copy so that if a fired functor changes m_simples/m_comparables iterators won't invalidate.
		auto simples = m_simples;
		auto comparables = m_comparables;
		lkg.unlock();

		for (auto& callee : simples) {
			callee(args...);
		}
		for (auto& callee : comparables) {
			callee(args...);
		}
	}

	/// <summary> Signs up functor for the event. You may remove this functor via <see cref="operator-="/>. </summary>
	template <class ComparableFun>
	std::enable_if_t<IsComparable<ComparableFun>, void> operator+=(const ComparableFun& fun) {
		std::lock_guard lkg(m_mtx);

		m_comparables.insert(Comparable(fun));
	}

	/// <summary> Signs up functor for the event. You can't remove this functor later. </summary>
	template <class SimpleFun>
	std::enable_if_t<!IsComparable<SimpleFun>, void> operator+=(SimpleFun fun) {
		std::function<void(ArgsT...)> callee = fun;

		std::lock_guard lkg(m_mtx);

		m_simples.push_back(callee);
	}

	/// <summary> Remove previously signed up functor. </summary>
	/// <returns> True if the functor was found and removed, false if not found. </returns>
	template <class ComparableFun>
	std::enable_if_t<IsComparable<ComparableFun>, bool> operator-=(const ComparableFun& fun) {
		std::lock_guard lkg(m_mtx);

		auto it = m_comparables.find(Comparable(fun));
		if (it != m_comparables.end()) {
			m_comparables.erase(it);
			return true;
		}
		return false;
	}

	/// <summary> Removes all signed up functors. </summary>
	void Clear() {
		std::lock_guard lkg(m_mtx);

		m_simples.clear();
		m_comparables.clear();
	}

private:
	mutable SpinMutex m_mtx;

	std::vector<std::function<void(ArgsT...)>> m_simples; // These functions cannot be removed via -=
	std::multiset<Comparable> m_comparables; // These function can be removed by -= because they have < and ==
};



} // namespace inl
