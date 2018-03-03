#include <functional>


namespace inl {


class AtScopeExit {
public:
	template <class Func>
	AtScopeExit(Func func, bool fused = true)
		: m_callable(std::move(func)), m_fused(fused)
	{}

	~AtScopeExit() {
		if (m_fused) {
			m_callable();
		}
	}

	void Fuse() {
		m_fused = true;
	}
	void Defuse() {
		m_fused = false;
	}
	void SetState(bool fused) {
		m_fused = fused;
	}
	bool IsFused() const {
		return m_fused;
	}
private:
	std::function<void()> m_callable;
	bool m_fused;
};


}