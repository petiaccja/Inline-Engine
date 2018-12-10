#pragma once

#include "Delegate.hpp"
#include "SpinMutex.hpp"
#include "TemplateUtil.hpp"

#include <set>
#include <mutex>
#include <vector>
#include <typeindex>
#include <functional>


#ifdef _MSC_VER
#pragma warning(disable: 4180)
#endif


namespace inl {


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
			compare = [](const std::function<void(ArgsT...)>& lhs, const std::function<void(ArgsT...)>& rhs) {
				const ComparableFun* target1 = lhs.target<const ComparableFun>();
				const ComparableFun* target2 = rhs.target<const ComparableFun>();
				if (target1 == nullptr || target2 == nullptr) {
					return
						((std::type_index)rhs.target_type() < (std::type_index)lhs.target_type())
						- ((std::type_index)lhs.target_type() < (std::type_index)rhs.target_type());
				}
				return (*target2 < *target1) - (*target1 < *target2);
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
		// Check less of underlying functors.
		bool operator<(const Comparable& rhs) const {
			return compare(callee, rhs.callee) < 0;
		}

		std::function<void(ArgsT...)> callee; // Underlying functor.
		std::function<int(const std::function<void(ArgsT...)>&, const std::function<void(ArgsT...)>&)> compare; // Comparison inner helper.
	};

	// Functors having both < and == are considered comparable.
	template <class Fun>
	static constexpr bool IsComparable = templ::is_equality_comparable<Fun>::value && templ::is_less_comparable<Fun>::value;
public:
	Event() = default;

	/// <summary> All signed up functors are copied as well and signed up for the new event. </summary>
	Event(const Event& other) : m_simples(other.m_simples), m_comparables(other.m_comparables) {}

	/// <summary> All signed up functors are copied as well and signed up for the new event, old event is empty. </summary>
	Event(Event&& other) : m_simples(std::move(other.m_simples)), m_comparables(std::move(other.m_comparables)) {}

	/// <summary> All signed up functors are copied as well and signed up for the new event. </summary>
	Event& operator=(const Event& other) {
		m_simples = other.m_simples;
		m_comparables = other.m_comparables;

		return *this;
	}

	/// <summary> All signed up functors are copied as well and signed up for the new event, old event is empty. </summary>
	Event& operator=(Event&& other) {
		m_simples = std::move(other.m_simples);
		m_comparables = std::move(other.m_comparables);

		return *this;
	}

	/// <summary> Fire off the event, call all signed up functors with given arguments. </summary>
	void operator()(ArgsT... args) {
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
		std::lock_guard<decltype(m_mtx)> lkg(m_mtx);
		
		m_comparables.insert(Comparable(fun));
	}

	/// <summary> Signs up functor for the event. You can't remove this functor later. </summary>
	template <class SimpleFun>
	std::enable_if_t<!IsComparable<SimpleFun>, void> operator+=(const SimpleFun& fun) {
		std::function<void(ArgsT...)> callee = fun;

		std::lock_guard<decltype(m_mtx)> lkg(m_mtx);

		m_simples.push_back(callee);
	}

	/// <summary> Remove previously signed up functor. </summary>
	/// <returns> True if the functor was found and removed, false if not found. </returns>
	template <class ComparableFun>
	std::enable_if_t<IsComparable<ComparableFun>, bool> operator-=(const ComparableFun& fun) {
		std::lock_guard<decltype(m_mtx)> lkg(m_mtx);

		auto it = m_comparables.find(Comparable(fun));
		if (it != m_comparables.end()) {
			m_comparables.erase(it);
			return true;
		}
		return false;
	}

private:
	SpinMutex m_mtx;

	std::vector<std::function<void(ArgsT...)>> m_simples; // These functions cannot be removed via -=
	std::multiset<Comparable> m_comparables; // These function can be removed by -= because they have < and ==
};



} // namespace inl
