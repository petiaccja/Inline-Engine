#include "Scene.hpp"
#include "System.hpp"

#include <compare>
#include <experimental/generator>
#include <vector>


namespace inl::game {


class Simulation {
	template <bool Const>
	class generic_iterator {
		friend generic_iterator<!Const>;
		using vector_iterator = std::conditional_t<Const, std::vector<std::unique_ptr<System>>::const_iterator, std::vector<std::unique_ptr<System>>::iterator>;

	public:
		generic_iterator() = default;
		generic_iterator(vector_iterator base) : m_it(base) {}

		generic_iterator(const generic_iterator& rhs) = default;
		template <class Dummy = void, class = std::enable_if_t<Const, Dummy>>
		generic_iterator(const generic_iterator<false>& rhs) : m_it(rhs.m_it) {}

		System& operator*() const { return **m_it; }
		System* operator->() const { return m_it->get(); }

		generic_iterator& operator++();
		generic_iterator& operator--();
		generic_iterator operator++(int);
		generic_iterator operator--(int);
		generic_iterator& operator+=(size_t n);
		generic_iterator& operator-=(size_t n);
		friend generic_iterator operator+(generic_iterator lhs, size_t n) { return lhs += n; }
		friend generic_iterator operator+(size_t n, generic_iterator rhs) { return rhs += n; }
		friend generic_iterator operator-(generic_iterator lhs, size_t n) { return lhs -= n; }
		friend size_t operator-(const generic_iterator& lhs, const generic_iterator& rhs) { return lhs.m_it - rhs.m_it; }
		auto operator<=>(const generic_iterator&) const = default;
		bool operator==(const generic_iterator&) const = default;

		vector_iterator get_underlying() const { return m_it; }

	private:
		vector_iterator m_it;
	};

public:
	using iterator = generic_iterator<false>;
	using const_iterator = generic_iterator<true>;

	void Run(Scene& scene, float elapsed);

	template <class SystemT>
	void Insert(const_iterator where, SystemT&& system);
	template <class SystemT>
	void PushBack(SystemT&& system);
	template <class SystemT>
	void RemoveAll();
	template <class SystemT>
	void Remove();
	void Remove(const System& system);
	void Clear();
	void Splice(const_iterator where, const_iterator which);
	size_t Size() const;

	System& operator[](size_t index);
	const System& operator[](size_t index) const;

	template <class SystemT>
	SystemT& Get();
	template <class SystemT>
	const SystemT& Get() const;

	template <class SystemT>
	std::experimental::generator<std::reference_wrapper<SystemT>> GetAll();
	template <class SystemT>
	std::experimental::generator<std::reference_wrapper<const SystemT>> GetAll() const;

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;


private:
	std::vector<std::unique_ptr<System>> m_systems;
};


template <bool Const>
Simulation::generic_iterator<Const>& Simulation::generic_iterator<Const>::operator++() {
	++m_it;
	return *this;
}

template <bool Const>
Simulation::generic_iterator<Const>& Simulation::generic_iterator<Const>::operator--() {
	--m_it;
	return *this;
}

template <bool Const>
Simulation::generic_iterator<Const> Simulation::generic_iterator<Const>::operator++(int) {
	return generic_iterator{ m_it++ };
}

template <bool Const>
Simulation::generic_iterator<Const> Simulation::generic_iterator<Const>::operator--(int) {
	return generic_iterator{ m_it-- };
}

template <bool Const>
Simulation::generic_iterator<Const>& Simulation::generic_iterator<Const>::operator+=(size_t n) {
	m_it += n;
	return *this;
}

template <bool Const>
Simulation::generic_iterator<Const>& Simulation::generic_iterator<Const>::operator-=(size_t n) {
	m_it -= n;
	return *this;
}


template <class SystemT>
void Simulation::Insert(const_iterator where, SystemT&& system) {
	m_systems.insert(where.get_underlying(), std::make_unique<SystemT>(std::forward<SystemT>(system)));
}


template <class SystemT>
void Simulation::PushBack(SystemT&& system) {
	Insert(end(), std::forward<SystemT>(system));
}


template <class SystemT>
void Simulation::RemoveAll() {
	auto keep = std::remove_if(m_systems.begin(), m_systems.end(), [](auto& ptr) {
		return typeid(SystemT) == typeid(*ptr);
	});
	m_systems.erase(keep, m_systems.end());
}


template <class SystemT>
void Simulation::Remove() {
	auto del = std::find_if(m_systems.begin(), m_systems.end(), [](auto& ptr) {
		return typeid(SystemT) == typeid(*ptr);
	});
	if (del != m_systems.end()) {
		m_systems.erase(del);
	}
}


template <class SystemT>
SystemT& Simulation::Get() {
	auto g = GetAll<SystemT>();
	auto first = g.begin();
	auto last = g.end();
	return first != last ? *first : throw OutOfRangeException("No systems of this type found.");
}


template <class SystemT>
const SystemT& Simulation::Get() const {
	auto g = GetAll<SystemT>();
	auto first = g.begin();
	auto last = g.end();
	return first != last ? *first : throw OutOfRangeException("No systems of this type found.");
}


template <class SystemT>
std::experimental::generator<std::reference_wrapper<SystemT>> Simulation::GetAll() {
	auto coro = [](decltype(m_systems)& systems) -> std::experimental::generator<std::reference_wrapper<SystemT>> {
		auto it = systems.begin();
		auto end = systems.end();
		while (it != end) {
			if (typeid(SystemT) == typeid(**it)) {
				co_yield dynamic_cast<SystemT&>(**it);
			}
			++it;
		}
	};
	return coro(m_systems);
}


template <class SystemT>
std::experimental::generator<std::reference_wrapper<const SystemT>> Simulation::GetAll() const {
	auto coro = [](const decltype(m_systems)& systems) -> std::experimental::generator<std::reference_wrapper<const SystemT>> {
		auto it = systems.begin();
		auto end = systems.end();
		while (it != end) {
			if (typeid(SystemT) == typeid(**it)) {
				co_yield dynamic_cast<const SystemT&>(**it);
			}
			++it;
		}
	};
	return coro(m_systems);
}



} // namespace inl::game
