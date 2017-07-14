#pragma once

#include <memory>

///////////////////////////////////////////////////////////////////////////
////////////////////////// DELEGATE, TODO REVIEW //////////////////////////
///////////////////////////////////////////////////////////////////////////
template <typename Signature>
struct Delegate;

template <typename... Args>
struct Delegate<void(Args...)>
{
	struct base {
		virtual ~base() {}
		//virtual bool do_cmp(base* other) = 0;
		virtual void do_call(Args... args) = 0;
	};
	template <typename T>
	struct call : base {
		T d_callback;
		template <typename S>
		call(S&& callback) : d_callback(std::forward<S>(callback)) {}

		//bool do_cmp(base* other) {
		//	call<T>* tmp = dynamic_cast<call<T>*>(other);
		//	return tmp && this->d_callback == tmp->d_callback;
		//}
		void do_call(Args... args) {
			return this->d_callback(std::forward<Args>(args)...);
		}
	};
	std::vector<std::shared_ptr<base>> d_callbacks;

	Delegate(Delegate const& other)
	{
		*this = other;
	}

	Delegate& operator=(Delegate const& other)
	{
		d_callbacks = other.d_callbacks;
		return *this;
	}

	operator bool() const
	{
		return d_callbacks.size() != 0;
	}

public:
	Delegate() {}
	template <typename T>
	Delegate& operator+= (T&& callback) {
		d_callbacks.emplace_back(new call<T>(std::forward<T>(callback)));
		//d_callbacks.emplace(d_callbacks.begin(), new call<T>(std::forward<T>(callback)));
		return *this;
	}
	//template <typename T>
	//Delegate& operator-= (T&& callback) {
	//	call<T> tmp(std::forward<T>(callback));
	//
	//	auto it = std::remove_if(d_callbacks.begin(), d_callbacks.end(), [&](std::unique_ptr<base>& other)
	//	{
	//		return tmp.do_cmp(other.get());
	//	});
	//	 
	//	this->d_callbacks.erase(it, this->d_callbacks.end());
	//	return *this;
	//}

	void operator()(Args... args)
	{
		// I'm sorry but it's necessary to copy the array, because callbacks can remove and add elements to them lol
		auto callbacks = d_callbacks;
		for (auto& callback : callbacks) {
			callback->do_call(args...);
		}
	}
};

template <typename RC, typename Class, typename... Args>
class MemberCall_ {
	Class* d_object;
	RC(Class::*d_member)(Args...);
public:
	MemberCall_(Class* object, RC(Class::*member)(Args...))
		: d_object(object)
		, d_member(member) {
	}
	RC operator()(Args... args) {
		return (this->d_object->*this->d_member)(std::forward<Args>(args)...);
	}
	bool operator== (MemberCall_ const& other) const {
		return this->d_object == other.d_object
			&& this->d_member == other.d_member;
	}
	bool operator!= (MemberCall_ const& other) const {
		return !(*this == other);
	}
};

template <typename RC, typename Class, typename... Args>
MemberCall_<RC, Class, Args...> MemberCall(Class& object,
	RC(Class::*member)(Args...)) {
	return MemberCall<RC, Class, Args...>(&object, member);
}
