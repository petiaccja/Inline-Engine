#pragma once

#include <cassert>
#include <cstring>
#include <new>
#include <typeindex>
#include <utility>


namespace inl {



template <class ReturnT, class... ArgsT>
class Delegate;



namespace impl {

	template <class ReturnT, class... ArgsT>
	class Callable;

	template <class... ArgsT>
	class CallableBase {
	public:
		virtual ~CallableBase() {}
		virtual void CallVoid(ArgsT... args) const = 0;
		virtual bool IsEmpty() const = 0;
		bool operator==(const CallableBase& rhs);
		bool operator!=(const CallableBase& rhs);
		bool operator<(const CallableBase& rhs);

	protected:
		virtual void* GetFuncPtr() const = 0;
		virtual size_t GetFuncPtrSize() const = 0;
		virtual bool IsClass() const = 0;
		virtual std::type_index GetClassType() const = 0;
		virtual void* GetClassPtr() const = 0;
	};

	template <class ReturnT, class... ArgsT>
	class Callable : public CallableBase<ArgsT...> {
	public:
		~Callable() {}
		void CallVoid(ArgsT... args) const override final {
			Call(std::forward<ArgsT>(args)...);
		}
		virtual ReturnT Call(ArgsT... args) const = 0;
	};

	template <class ReturnT, class... ArgsT>
	class GlobalCallable : public Callable<ReturnT, ArgsT...> {
	public:
		GlobalCallable(ReturnT (*func)(ArgsT...)) : m_func(func) {}
		~GlobalCallable() {}
		bool IsEmpty() const override { return m_func == nullptr; }
		void* GetFuncPtr() const override { return m_func; }
		size_t GetFuncPtrSize() const override { return sizeof(m_func); }
		bool IsClass() const override { return false; }
		std::type_index GetClassType() const override { return typeid(void); }
		void* GetClassPtr() const override { return nullptr; }
		ReturnT Call(ArgsT... args) const override {
			assert(m_func != nullptr);
			return m_func(std::forward<ArgsT>(args)...);
		}

	private:
		ReturnT (*m_func)(ArgsT...) = nullptr;
	};

	template <class ClassT, class ReturnT, class... ArgsT>
	class MemberCallable : public Callable<ReturnT, ArgsT...> {
	public:
		using FuncT = typename std::conditional<std::is_const<ClassT>::value, ReturnT (ClassT::*)(ArgsT...) const, ReturnT (ClassT::*)(ArgsT...)>::type;
		MemberCallable(FuncT func, ClassT* cl) : m_func(func), m_class(cl) {}
		~MemberCallable() {}
		bool IsEmpty() const override { return m_func == nullptr; }
		void* GetFuncPtr() const override { return (void*)&m_func; }
		size_t GetFuncPtrSize() const override { return sizeof(m_func); }
		bool IsClass() const override { return true; }
		std::type_index GetClassType() const override { return typeid(ClassT); }
		void* GetClassPtr() const override { return m_class; }
		ReturnT Call(ArgsT... args) const override {
			assert(m_func != nullptr && m_class != nullptr);
			return (m_class->*m_func)(std::forward<ArgsT>(args)...);
		}

	private:
		FuncT m_func = nullptr;
		ClassT* m_class = nullptr;
	};



} // namespace impl



template <class ReturnT, class... ArgsT>
class Delegate<ReturnT(ArgsT...)> {
	class Dummy {};

public:
	Delegate() = default;
	Delegate(const Delegate& rhs) {
		memcpy(m_callablePlaceholder, rhs.m_callablePlaceholder, sizeof(m_callablePlaceholder));
		m_callable = reinterpret_cast<impl::Callable<void, ArgsT...>*>(m_callablePlaceholder + ((size_t)rhs.m_callable - (size_t)rhs.m_callablePlaceholder));
	}
	Delegate& operator=(const Delegate& rhs) {
		memcpy(m_callablePlaceholder, rhs.m_callablePlaceholder, sizeof(m_callablePlaceholder));
		m_callable = reinterpret_cast<impl::Callable<void, ArgsT...>*>(m_callablePlaceholder + ((size_t)rhs.m_callable - (size_t)rhs.m_callablePlaceholder));
		return *this;
	}
	Delegate(Delegate&&) = delete;
	Delegate& operator=(Delegate&&) = delete;

	Delegate(ReturnT (*func)(ArgsT...)) {
		static_assert(sizeof(impl::GlobalCallable<ReturnT, ArgsT...>) <= sizeof(m_callablePlaceholder), "Tell this error and the compiler you used to the author.");
		new (m_callablePlaceholder) impl::GlobalCallable<ReturnT, ArgsT...>(func);
		m_callable = static_cast<impl::Callable<ReturnT, ArgsT...>*>(reinterpret_cast<impl::GlobalCallable<ReturnT, ArgsT...>*>(m_callablePlaceholder));
	}

	template <class ClassT, typename std::enable_if<!std::is_const<ClassT>::value, int>::type = 0>
	Delegate(ReturnT (ClassT::*func)(ArgsT...), ClassT* owner) {
		static_assert(sizeof(impl::MemberCallable<ClassT, ReturnT, ArgsT...>) <= sizeof(m_callablePlaceholder), "Tell this error and the compiler you used to the author.");
		new (m_callablePlaceholder) impl::MemberCallable<ClassT, ReturnT, ArgsT...>(func, owner);
		m_callable = static_cast<impl::Callable<ReturnT, ArgsT...>*>(reinterpret_cast<impl::MemberCallable<ClassT, ReturnT, ArgsT...>*>(m_callablePlaceholder));
	}

	template <class ClassT>
	Delegate(ReturnT (ClassT::*func)(ArgsT...) const, const ClassT* owner) {
		static_assert(sizeof(impl::MemberCallable<ClassT, ReturnT, ArgsT...>) <= sizeof(m_callablePlaceholder), "Tell this error and the compiler you used to the author.");
		new (m_callablePlaceholder) impl::MemberCallable<ClassT, ReturnT, ArgsT...>(func, owner);
		m_callable = static_cast<impl::Callable<ReturnT, ArgsT...>*>(reinterpret_cast<impl::MemberCallable<ClassT, ReturnT, ArgsT...>*>(m_callablePlaceholder));
	}

	explicit operator bool() const {
		return m_callable && !m_callable->IsEmpty();
	}

	bool operator==(const Delegate& rhs) const {
		if (!m_callable || !rhs.m_callable) {
			return false;
		}
		return *m_callable == *rhs.m_callable;
	}
	bool operator!=(const Delegate& rhs) const {
		return !(*this == rhs);
	}
	bool operator<(const Delegate& rhs) const {
		if (!m_callable) {
			return rhs.m_callable != nullptr;
		}
		if (rhs.m_callable) {
			return *m_callable < *rhs.m_callable;
		}
		return false;
	}

	ReturnT operator()(ArgsT... args) const {
		assert(operator bool());
		return m_callable->Call(std::forward<ArgsT>(args)...);
	}

private:
	alignas(impl::MemberCallable<Dummy, ReturnT, ArgsT...>) char m_callablePlaceholder[sizeof(impl::MemberCallable<Dummy, ReturnT, ArgsT...>)]; // actual stuff is placement newed here
	inl::impl::Callable<ReturnT, ArgsT...>* m_callable = nullptr;
};



template <class... ArgsT>
class Delegate<void(ArgsT...)> {
	class DummyBase1 {
		virtual ~DummyBase1() {}
	};
	class DummyBase2 {
		virtual ~DummyBase2() {}
	};
	class Dummy : virtual public DummyBase1, public virtual DummyBase2 {
		virtual ~Dummy() {}
	};

public:
	Delegate() = default;
	Delegate(const Delegate& rhs) {
		memcpy(m_callablePlaceholder, rhs.m_callablePlaceholder, sizeof(m_callablePlaceholder));
		m_callable = reinterpret_cast<impl::Callable<void, ArgsT...>*>(m_callablePlaceholder + ((size_t)rhs.m_callable - (size_t)rhs.m_callablePlaceholder));
	}
	Delegate& operator=(const Delegate& rhs) {
		memcpy(m_callablePlaceholder, rhs.m_callablePlaceholder, sizeof(m_callablePlaceholder));
		m_callable = reinterpret_cast<impl::Callable<void, ArgsT...>*>(m_callablePlaceholder + ((size_t)rhs.m_callable - (size_t)rhs.m_callablePlaceholder));
		return *this;
	}

	template <class ReturnT>
	Delegate(const Delegate<ReturnT, ArgsT...>& rhs) {
		memcpy(m_callablePlaceholder, rhs.m_callablePlaceholder, sizeof(m_callablePlaceholder));
		m_callable = reinterpret_cast<impl::Callable<ReturnT, ArgsT...>*>(m_callablePlaceholder + ((size_t)rhs.m_callable - (size_t)rhs.m_callablePlaceholder));
	}

	template <class ReturnT>
	Delegate& operator=(const Delegate<ReturnT, ArgsT...>& rhs) {
		memcpy(m_callablePlaceholder, rhs.m_callablePlaceholder, sizeof(m_callablePlaceholder));
		m_callable = reinterpret_cast<impl::Callable<ReturnT, ArgsT...>*>(m_callablePlaceholder + ((size_t)rhs.m_callable - (size_t)rhs.m_callablePlaceholder));
		return *this;
	}
	Delegate(Delegate&& rhs) : Delegate((const Delegate&)rhs) {}
	Delegate& operator=(Delegate&& rhs) { return *this = (const Delegate&)rhs; }

	template <class ReturnT>
	Delegate(ReturnT (*func)(ArgsT...)) {
		static_assert(sizeof(impl::GlobalCallable<ReturnT, ArgsT...>) <= sizeof(m_callablePlaceholder), "Tell this error and the compiler you used to the author.");
		new (m_callablePlaceholder) impl::GlobalCallable<ReturnT, ArgsT...>(func);
		m_callable = static_cast<impl::Callable<ReturnT, ArgsT...>*>(reinterpret_cast<impl::GlobalCallable<ReturnT, ArgsT...>*>(m_callablePlaceholder));
	}

	template <class ClassT, class ReturnT, typename std::enable_if<!std::is_const<ClassT>::value, int>::type = 0>
	Delegate(ReturnT (ClassT::*func)(ArgsT...), ClassT* owner) {
		static_assert(sizeof(impl::MemberCallable<ClassT, ReturnT, ArgsT...>) <= sizeof(m_callablePlaceholder), "Tell this error and the compiler you used to the author.");
		new (m_callablePlaceholder) impl::MemberCallable<ClassT, ReturnT, ArgsT...>(func, owner);
		m_callable = static_cast<impl::Callable<ReturnT, ArgsT...>*>(reinterpret_cast<impl::MemberCallable<ClassT, ReturnT, ArgsT...>*>(m_callablePlaceholder));
	}

	template <class ClassT, class ReturnT>
	Delegate(ReturnT (ClassT::*func)(ArgsT...) const, const ClassT* owner) {
		static_assert(sizeof(impl::MemberCallable<ClassT, ReturnT, ArgsT...>) <= sizeof(m_callablePlaceholder), "Tell this error and the compiler you used to the author.");
		new (m_callablePlaceholder) impl::MemberCallable<ClassT, ReturnT, ArgsT...>(func, owner);
		m_callable = static_cast<impl::Callable<ReturnT, ArgsT...>*>(reinterpret_cast<impl::MemberCallable<ClassT, ReturnT, ArgsT...>*>(m_callablePlaceholder));
	}

	explicit operator bool() const {
		return m_callable && !m_callable->IsEmpty();
	}

	bool operator==(const Delegate& rhs) const {
		if (!m_callable || !rhs.m_callable) {
			return false;
		}
		return *m_callable == *rhs.m_callable;
	}
	bool operator!=(const Delegate& rhs) const {
		return !(*this == rhs);
	}
	bool operator<(const Delegate& rhs) const {
		if (!m_callable) {
			return rhs.m_callable != nullptr;
		}
		if (rhs.m_callable) {
			return *m_callable < *rhs.m_callable;
		}
		return false;
	}

	void operator()(ArgsT... args) const {
		assert(operator bool());
		m_callable->CallVoid(std::forward<ArgsT>(args)...);
	}

private:
	alignas(impl::MemberCallable<Dummy, void, ArgsT...>) char m_callablePlaceholder[sizeof(impl::MemberCallable<Dummy, void, ArgsT...>)]; // actual stuff is placement newed here
	impl::CallableBase<ArgsT...>* m_callable = nullptr;
};



template <class... ArgsT>
bool impl::CallableBase<ArgsT...>::operator==(const CallableBase& rhs) {
	if (IsClass() && rhs.IsClass()) {
		return GetClassPtr() == rhs.GetClassPtr()
			   && GetClassType() == rhs.GetClassType()
			   && GetFuncPtrSize() == rhs.GetFuncPtrSize()
			   && 0 == memcmp(GetFuncPtr(), rhs.GetFuncPtr(), GetFuncPtrSize());
	}
	else if (!IsClass() && !rhs.IsClass()) {
		return GetFuncPtr() == rhs.GetFuncPtr();
	}
	else {
		return false;
	}
}

template <class... ArgsT>
bool impl::CallableBase<ArgsT...>::operator!=(const CallableBase& rhs) {
	return !(*this == rhs);
}

template <class... ArgsT>
bool impl::CallableBase<ArgsT...>::operator<(const CallableBase& rhs) {
	if (IsClass() && rhs.IsClass()) {
		// observe angle bracket direction
		unsigned ct = (GetClassType() == rhs.GetClassType()) + (GetClassType() > rhs.GetClassType());
		unsigned cp = (GetClassPtr() == rhs.GetClassPtr()) + (GetClassPtr() > rhs.GetClassPtr());
		unsigned fs = (GetFuncPtrSize() == rhs.GetFuncPtrSize()) + (GetFuncPtrSize() > rhs.GetFuncPtrSize());
		unsigned fp = 1;
		if (fs == 1) {
			int cres = memcmp(GetFuncPtr(), rhs.GetFuncPtr(), GetFuncPtrSize());
			fp = cres < 0 ? 0 : (cres == 0 ? 1 : 2);
		}

		// notice how the angle brackets are turned around
		unsigned rct = (GetClassType() == rhs.GetClassType()) + (GetClassType() < rhs.GetClassType());
		unsigned rcp = (GetClassPtr() == rhs.GetClassPtr()) + (GetClassPtr() < rhs.GetClassPtr());
		unsigned rfs = (GetFuncPtrSize() == rhs.GetFuncPtrSize()) + (GetFuncPtrSize() < rhs.GetFuncPtrSize());
		unsigned rfp = 1;
		if (rfs == 1) {
			int cres = memcmp(rhs.GetFuncPtr(), GetFuncPtr(), GetFuncPtrSize());
			rfp = cres < 0 ? 0 : (cres == 0 ? 1 : 2);
		}

		// all are represented by two bit, hence the 2 incerement in bitshift (values 0,1,2)
		unsigned l = (ct << 6) | (cp << 4) | (fs << 2) | fp;
		unsigned r = (rct << 6) | (rcp << 4) | (rfs << 2) | rfp;

		return l < r;
	}
	else if (!IsClass() && !rhs.IsClass()) {
		return GetFuncPtr() < rhs.GetFuncPtr();
	}
	else {
		return IsClass() < rhs.IsClass();
	}
}



} // namespace inl
