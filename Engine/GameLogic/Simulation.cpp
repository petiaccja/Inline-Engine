#include "Simulation.hpp"


namespace inl::game {


void Simulation::Run(Scene& scene, float elapsed) {
	for (auto& system : *this) {
		const auto& systemScheme = system.Scheme();
		if (systemScheme.Empty()) {
			system.Update(elapsed);
		}
		else {
			for (auto& matrix : scene.GetMatrices(systemScheme)) {
				system.Update(elapsed, matrix);
			}
		}
	}
}

void Simulation::Remove(const System& system) {
	auto keep = std::remove_if(m_systems.begin(), m_systems.end(), [&system](const auto& ptr) {
		return ptr.get() == &system;
	});
	m_systems.erase(keep, m_systems.end());
}

void Simulation::Clear() {
	m_systems.clear();
}

void Simulation::Splice(const_iterator where, const_iterator which) {
	auto vwhere = where.get_underlying();
	auto vwhich = m_systems.begin() + (which.get_underlying() - m_systems.cbegin());
	auto ptr = std::move(*vwhich);
	m_systems.erase(vwhich);
	m_systems.insert(vwhere, std::move(ptr));
}

size_t Simulation::Size() const {
	return m_systems.size();
}

System& Simulation::operator[](size_t index) {
	return *(begin() + index);
}

const System& Simulation::operator[](size_t index) const {
	return *(begin() + index);
}

Simulation::iterator Simulation::begin() {
	return iterator{ m_systems.begin() };
}

Simulation::iterator Simulation::end() {
	return iterator{ m_systems.end() };
}

Simulation::const_iterator Simulation::begin() const {
	return const_iterator{ m_systems.begin() };
}

Simulation::const_iterator Simulation::end() const {
	return const_iterator{ m_systems.end() };
}

Simulation::const_iterator Simulation::cbegin() const {
	return const_iterator{ m_systems.cbegin() };
}

Simulation::const_iterator Simulation::cend() const {
	return const_iterator{ m_systems.cend() };
}


} // namespace inl::game
