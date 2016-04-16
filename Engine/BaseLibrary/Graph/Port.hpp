#pragma once

#include <set>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <functional>
#include <iterator>

namespace exc {

class InputPortBase;
class OutputPortBase;
class NodeBase;


/// <summary> Special type to parametrize Ports with.
/// Allows this kind of port to be connected to any type. </summary>
class AnyType {
};



/// <summary>
/// <para> Input port of a Node. </para>
/// <para> 
/// Input ports are attached to a node. An input port can also be linked
/// to an output port, from where it receives the data.
/// </para>
/// </summary>
class InputPortBase {
	friend class OutputPortBase;
public:
	InputPortBase();
	~InputPortBase();

	/// <summary> Clear currently stored data. </summary>
	virtual void Clear() = 0;

	/// <summary> Get weather any valid data is set. </summary>
	virtual bool IsSet() const = 0;

	/// <summary> Get typeid of underlying data. </summary>
	virtual std::type_index GetType() const = 0;

	/// <summary> Link this port to an output port. </summary>
	/// <returns> True if succesfully linked. Make sures types are compatible. </returns>
	bool Link(OutputPortBase* source);

	/// <summary> Remove link between this and the other end. </summary>
	void Unlink();

	/// <summary> Add observer node.
	/// Observers are notified when new data is set. </summary>
	virtual void AddObserver(NodeBase* observer) final;
	/// <summary> Remove observer. </summary>
	virtual void RemoveObserver(NodeBase* observer) final;

	/// <summary> Get which output port it is linked to. </summary>
	/// <returns> The other end. Null if not linked. </returns>
	OutputPortBase* GetLink() const;
protected:
	OutputPortBase* link;
	void NotifyAll();

private:
	// should only be called by an output port when it's ready with building up the linkage
	// this function only sets internal state of the inputport to represent the link set up by outputport
	void SetLinkState(OutputPortBase* link);

	std::set<NodeBase*> observers;
};



/// <summary>
/// <para> Output port of a node. </para>
/// <para>
/// Output ports are attached to nodes. They can be linked to
/// input ports. A node can activate them with data, and that data
/// is forwarded to connected input ports. An output port can be linked
/// to multiple input ports at the same time.
/// </para>
class OutputPortBase {
private:
	friend class LinkIteratorBase;

	template <class T>
	class LinkIteratorBase : public std::iterator<std::bidirectional_iterator_tag, T> {
	private:
		using ContainerIterator = typename std::conditional<std::is_const<T>::value, std::set<InputPortBase*>::const_iterator, std::set<InputPortBase*>::iterator>::type;
		using ParentType = typename std::conditional<std::is_const<T>::value, const OutputPortBase, OutputPortBase>::type;

		friend class OutputPortBase;
		LinkIteratorBase(ParentType* parent, bool isBegin);
	public:
		LinkIteratorBase();
		LinkIteratorBase(const LinkIteratorBase&) = default;
		LinkIteratorBase& operator=(const LinkIteratorBase&) = default;

		template <class U = const LinkIteratorBase<typename std::enable_if<!std::is_const<T>::value, const T>::type>>
		LinkIteratorBase(U& rhs);
		template <class U = const LinkIteratorBase<typename std::enable_if<!std::is_const<T>::value, const T>::type>>
		LinkIteratorBase& operator=(U& rhs);

		T& operator*();
		T* operator->();

		bool operator==(const LinkIteratorBase&);
		bool operator!=(const LinkIteratorBase&);

		LinkIteratorBase& operator++();
		LinkIteratorBase operator++(int);
		LinkIteratorBase& operator--();
		LinkIteratorBase operator--(int);
	private:
		ParentType* parent;
		std::set<InputPortBase*>::iterator containerIt;
		int currentContainer;
		static constexpr int NORMAL = 1;
		static constexpr int ANYLINK = 2;
	};
public:
	using LinkIterator = LinkIteratorBase<InputPortBase>;
	using ConstLinkIterator = LinkIteratorBase<const InputPortBase>;
public:
	OutputPortBase();
	~OutputPortBase();

	/// <summary> Get typeid of underlying data. </summary>
	virtual std::type_index GetType() const = 0;

	/// <summary> Link to an input port. </summary>
	/// <returns> True if succesfully linked. Make sures types are compatible. </returns>
	virtual bool Link(InputPortBase* destination);

	/// <summary> Remove link between this and the other end. </summary>
	/// <param param="other"> The port to unlink from this. </param>
	virtual void Unlink(InputPortBase* other);

	/// <summary> Unlink all ports from this. </summary>
	virtual void UnlinkAll();

	//! TODO: add iterator support to iterate over links
	LinkIterator begin();
	LinkIterator end();
	ConstLinkIterator begin() const;
	ConstLinkIterator end() const;
	ConstLinkIterator cbegin() const;
	ConstLinkIterator cend() const;
protected:
	std::set<InputPortBase*> links;
	std::set<InputPortBase*> anyLinks;
};



template <class T>
OutputPortBase::LinkIteratorBase<T>::LinkIteratorBase(ParentType* parent, bool isBegin) : parent(parent) {
	if (isBegin) {
		currentContainer = NORMAL;
		containerIt = parent->links.begin();
		if (containerIt == parent->links.end()) {
			containerIt = parent->anyLinks.begin();
			currentContainer = ANYLINK;
		}
	}
	else {
		currentContainer = ANYLINK;
		containerIt = parent->anyLinks.end();
	}
}
template <class T>
OutputPortBase::LinkIteratorBase<T>::LinkIteratorBase() {
	currentContainer = ANYLINK;
	parent = nullptr;
}

template <class T>
template <class U>
OutputPortBase::LinkIteratorBase<T>::LinkIteratorBase(U& rhs) {
	parent = rhs.parent;
	currentContainer = rhs.currentContainer;
	containerIt = rhs.containerIt;
}
template <class T>
template <class U>
OutputPortBase::LinkIteratorBase<T>& OutputPortBase::LinkIteratorBase<T>::operator=(U& rhs) {
	parent = rhs.parent;
	currentContainer = rhs.currentContainer;
	containerIt = rhs.containerIt;
}

template <class T>
T& OutputPortBase::LinkIteratorBase<T>::operator*() {
	return **containerIt;
}

template <class T>
T* OutputPortBase::LinkIteratorBase<T>::operator->() {
	return *containerIt;
}

template <class T>
bool OutputPortBase::LinkIteratorBase<T>::operator==(const LinkIteratorBase& rhs) {
	return (parent == rhs.parent && currentContainer == rhs.currentContainer && containerIt == rhs.containerIt);
}

template <class T>
bool OutputPortBase::LinkIteratorBase<T>::operator!=(const LinkIteratorBase& rhs) {
	return *this != rhs;
}

template <class T>
OutputPortBase::LinkIteratorBase<T>& OutputPortBase::LinkIteratorBase<T>::operator++() {
	containerIt++;
	if (currentContainer == NORMAL && containerIt == parent->links.end()) {
		currentContainer = ANYLINK;
		containerIt = parent->anylinks.begin();
	}
	return *this;
}

template <class T>
OutputPortBase::LinkIteratorBase<T>  OutputPortBase::LinkIteratorBase<T>::operator++(int) {
	auto copy = *this;
	++(*this);
	return copy;
}

template <class T>
OutputPortBase::LinkIteratorBase<T>& OutputPortBase::LinkIteratorBase<T>::operator--() {
	if (currentContainer == ANYLINK && containerIt == parent->anyLinks.begin()) {
		currentContainer = NORMAL;
		containerIt = --(parent->links.end());
	}
	else {
		--containerIt;
	}
	return *this;
}

template <class T>
OutputPortBase::LinkIteratorBase<T>  OutputPortBase::LinkIteratorBase<T>::operator--(int) {
	auto copy = *this;
	--(*this);
	return copy;
}


/// <summary>
/// <para> Specialization of InputPortBase for various types of data. </para>
/// <para> Different types can be set as template parameter. Generally, it's enough to
/// just use this template, but it may be necessary to specialize this template
/// for certain data types to improve efficiency or change behaviour. </para>
/// </summary>

template <class T>
class InputPort : public InputPortBase {
public:
	InputPort() {
		isSet = false;
	}
	InputPort(const InputPort&) = default;
	InputPort(InputPort&&) = default;
	InputPort& operator=(const InputPort&) = default;
	InputPort& operator=(InputPort&&) = default;


	/// <summary> 
	/// Set an object as input to this port.
	/// This is normally called by linked output ports, but may as well be
	/// called manually.
	/// </summary>
	void Set(const T& data) {
		this->data = data;
		isSet = true;
		NotifyAll();
	}

	/// <summary> 
	/// Get the data that was previously set.
	/// If no data is set, the behaviour is undefined.
	/// </summary>
	/// <returns> Reference to the data currently set. </returns>
	T& Get() {
		return data;
	}

	/// <summary> 
	/// Get the data that was previously set.
	/// If no data is set, the behaviour is undefined.
	/// </summary>
	/// <returns> Reference to the data currently set. </returns>
	const T& Get() const {
		return data;
	}

	/// <summary> Clear any data currently set on this port. </summary>
	void Clear() override {
		isSet = false;
		data = T();
	}

	/// <summary> Get whether any data has been set. </summary>
	bool IsSet() const override {
		return isSet;
	}

	/// <summary> Get the underlying data type. </summary>
	std::type_index GetType() const override {
		return typeid(T);
	}

private:
	bool isSet;
	T data;
};



/// <summary>
/// Specialization of OutputPortBase for various types of data.
/// Different types can be set as template parameter. Generally, it's enough to
/// just use this template, but it may be necessary to specialize this template
/// for certain data types to improve efficiency or change behaviour.
/// </summary>
template <class T>
class OutputPort : public OutputPortBase {
public:
	// ctors
	OutputPort() {

	}
	OutputPort(const OutputPort&) = default;
	OutputPort(OutputPort&&) = default;
	OutputPort& operator=(const OutputPort&) = default;
	OutputPort& operator=(OutputPort&&) = default;


	/// <summary> Set data on this port.
	/// This data is forwarded to each input port linked to this one. </summary>
	void Set(const T& data);

	/// <summary> Get type of underlying data. </summary>
	std::type_index GetType() const override {
		return typeid(T);
	}
};



////////////////////////////////////////////////////////////////////////////////
/// Specialization for void type ports
////////////////////////////////////////////////////////////////////////////////
template <>
class InputPort<void> : public InputPortBase {
public:
	// ctors
	InputPort() {
		isSet = false;
	}
	InputPort(const InputPort&) = default;
	InputPort(InputPort&&) = default;
	InputPort& operator=(const InputPort&) = default;
	InputPort& operator=(InputPort&&) = default;


	/// Set an object as input to this port.
	/// This is normally called by linked output ports, but may as well be
	/// called manually.
	void Set() {
		isSet = true;
		NotifyAll();
	}

	/// Clear any data currently set on this port.
	void Clear() override {
		isSet = false;
	}

	/// Get whether any data has been set.
	bool IsSet() const override {
		return isSet;
	}

	/// Get the underlying data type.
	std::type_index GetType() const override {
		return typeid(void);
	}
private:
	bool isSet;
};



////////////////////////////////////////////////////////////////////////////////
/// Specialization for void type ports.
////////////////////////////////////////////////////////////////////////////////
template <>
class OutputPort<void> : public OutputPortBase {
public:
	// ctors
	OutputPort() {

	}
	OutputPort(const OutputPort&) = default;
	OutputPort(OutputPort&&) = default;
	OutputPort& operator=(const OutputPort&) = default;
	OutputPort& operator=(OutputPort&&) = default;


	/// Set data on this port.
	/// This data is forwarded to each input port linked to this one.
	void Set();

	/// Get type of underlying data.
	std::type_index GetType() const override {
		return typeid(void);
	}
};



////////////////////////////////////////////////////////////////////////////////
/// A special object which is used with AnyType ports.
////////////////////////////////////////////////////////////////////////////////
class AnyTypeData {
	friend class OutputPort<AnyType>;
public:
	/// Get pointer to stored object's memory layout.
	/// \return Pointer to the stored object, null if empty.
	virtual void* Get() = 0;

	/// Get const pointer to stored object's memory layout.
	/// \return Pointer to the stored object, null if empty.
	virtual const void* Get() const = 0;

	/// Get the size of the stored object.
	/// \return Size of the object in bytes, 0 if empty.
	virtual size_t Size() const = 0;

	virtual ~AnyTypeData() = default;
private:
	// Try to set stored object on an input port. If target has
	// incompatible type, false is returned.
	virtual bool SendToInputport(InputPortBase* target) const = 0;
};


////////////////////////////////////////////////////////////////////////////////
/// Template specialization for AnyType ports.
////////////////////////////////////////////////////////////////////////////////
template <>
class InputPort<AnyType> : public InputPortBase {
public:
	InputPort() : currentType(typeid(AnyType)), data(nullptr) {
	}

	/// Set data as input.
	/// As a template function, it accepts any type of data.
	/// \return Returns true, always. Should return false if type constraints \
	/// don't allow insertion of data, but type constraints are not implemented yet.
	template <class U>
	bool Set(const U& data) {
		// AnyType is an invalid type
		static_assert(!std::is_same<U, AnyType>::value, "AnyType is not a valid data type for Set");

		// check if U equals currentType
		// can't set mismatching type!
		// TYPE CONSTRAINT NOT IMPLEMENTED YET
		//if (currentType != typeid(U)) {
		//	//return false;
		//}

		// delete old/previous data if any
		Clear();

		// create new data
		class AnyTypeDataSpec : public AnyTypeData {
		public:
			AnyTypeDataSpec() = default;
			AnyTypeDataSpec(const U& data) : data(data) {};

			void* Get() override { return reinterpret_cast<void*>(&data); }
			const void* Get() const override { return reinterpret_cast<const void*>(&data); }
			size_t Size() const override { return sizeof(data); }
		private:
			bool SendToInputport(InputPortBase* target) const override {
				if (target->GetType() == typeid(AnyType)) {
					InputPort<AnyType>* targetSpec = static_cast<InputPort<AnyType>*>(target);
					return targetSpec->Set(data);
				}
				else {
					InputPort<U>* targetSpec = dynamic_cast<InputPort<U>*>(target);
					if (targetSpec != nullptr) {
						targetSpec->Set(data);
						return true;
					}
					else {
						return false;
					}
				}
			}
			U data;
		};
		this->data = new AnyTypeDataSpec{ data };

		NotifyAll();
		return true;
	}

	/// Set void data.
	/// May be called by void type ports.
	bool Set() {
		Clear();

		class AnyTypeDataVoid : public AnyTypeData {
		public:
			virtual void* Get() {
				return nullptr;
			}
			virtual const void* Get() const {
				return nullptr;
			}
			virtual size_t Size() const {
				return 0;
			}
		private:
			virtual bool SendToInputport(InputPortBase* target) const {
				if (target->GetType() == typeid(AnyType)) {
					InputPort<AnyType>* targetSpec = static_cast<InputPort<AnyType>*>(target);
					return targetSpec->Set();
				}
				else if (target->GetType() == typeid(void)) {
					InputPort<void>* targetSpec = static_cast<InputPort<void>*>(target);
					targetSpec->Set();
					return true;
				}
				else {
					return false;
				}
			}
		};
		this->data = new AnyTypeDataVoid();

		NotifyAll();
		return true;
	}

	/// Get stored data.
	/// \return Pointer to a special object constaining the data. \
	/// Null if no data is stored.
	AnyTypeData* Get() {
		return data;
	}
	/// Get stored data.
	/// \return Pointer to a special object constaining the data. \
	/// Null if no data is stored.
	const AnyTypeData* Get() const {
		return data;
	}

	/// Clear currently set data.
	void Clear() override {
		if (data) {
			delete data;
			data = nullptr;
		}
	}

	/// Check if there's any data set.
	bool IsSet() const override {
		return data != nullptr;
	}
	/// Get type of this port.
	/// \return Always typeid(AnyType).
	std::type_index GetType() const override {
		return typeid(AnyType);
	}
	/// Get current type.
	/// Used by type constraints, which are not implemented yet, so it's irrelevant.
	std::type_index GetCurrentType() const {
		return currentType;
	}

private:
	AnyTypeData* data;
	std::type_index currentType;
};


////////////////////////////////////////////////////////////////////////////////
/// Template specialization for AnyType ports.
////////////////////////////////////////////////////////////////////////////////
template <>
class OutputPort<AnyType> : public OutputPortBase {
public:
	OutputPort() : currentType(typeid(AnyType)) {}

	/// Set data on this output port.
	/// Keep in mind that mismatching input will NOT be forwarded to input ports
	/// linked to this output port. For example, a recieved "float" type won't
	/// be sent to a linked "int" type input port.
	/// \param data A special object which contains the real data.
	void Set(const AnyTypeData& data) {
		for (auto v : links) {
			data.SendToInputport(v);
		}
		for (auto v : anyLinks) {
			data.SendToInputport(v);
		}
	}

	/// Get type of this port.
	/// \return Always typeid(AnyType).
	std::type_index GetType() const override {
		return typeid(AnyType);
	}
	/// Get current type.
	/// Used by type constraints, which are not implemented yet, so it's irrelevant.
	std::type_index GetCurrentType() const {
		return currentType;
	}
private:
	std::type_index currentType;
};


template <class T>
void OutputPort<T>::Set(const T& data) {
	for (auto v : links) {
		static_cast<InputPort<T>*>(v)->Set(data);
	}
	for (auto v : anyLinks) {
		static_cast<InputPort<AnyType>*>(v)->Set(data);
	}
}



// rough prototype for enforcing types of AnyType ports
// type constraint is not that easy so i'll keep it for later

// cases:
// OUTPUT	-	INPUT
// anytype	-	anytype
// t		-	anytype
// anytype	-	t
/*
class TypeConstraint {
public:
	TypeConstraint(OutputPort<AnyType>& op, InputPort<AnyType>& ip) {

	}

	TypeConstraint(OutputPort<AnyType>& op, InputPortBase& ip) {
		// redirect double-anytype calls
		if (ip.GetType() == typeid(AnyType)) {
			new (this) TypeConstraint(op, static_cast<InputPort<AnyType>&>(ip));
			return;
		}

		// create constraint
	}

	TypeConstraint(OutputPortBase& op, InputPort<AnyType>& ip) {
		// redirect double-anytype calls
		if (op.GetType() == typeid(AnyType)) {
			new (this) TypeConstraint(static_cast<OutputPort<AnyType>&>(op), ip);
			return;
		}

		// create constraint
	}
};
*/

// explicit instantiations
extern template class InputPort<AnyType>;
extern template class OutputPort<AnyType>;

} // namespace exc