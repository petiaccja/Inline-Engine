#pragma once

#include <set>
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#include <functional>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <sstream>

#include "../Any.hpp"
#include "../TemplateUtil.hpp"
#include "SerializableNode.hpp"


namespace inl {

class InputPortBase;
class OutputPortBase;
class NodeBase;

template <class T>
class OutputPort;

template <class T, class C>
class InputPort;




/// <summary> Use this as a helper to implement <see cref="PortConverter"/> specializations.
/// <para> Inherit from this class, then pass the conversion functions to its constructor.
///		The conversion functions should not be member functions, and must have the signature
///		DestinationType(const SourceType&amp;). Argument can be passed by const-ref or value. </para>
///	</summary>
template <class T>
class PortConverterCollection {
public:
	template <class... Functions>
	explicit PortConverterCollection(Functions... functions) {
		RegisterFunctions(functions...);
	}

	auto operator[](std::type_index sourceType) const -> const std::function<void(const void*, void*)>& {
		auto it = m_converters.find(sourceType);
		if (it != m_converters.end()) {
			return it->second;
		}
		else {
			throw OutOfRangeException("Cannot find a converter for this type.", sourceType.name());
		}
	}

	bool CanConvert(std::type_index type) const {
		return m_converters.count(type) > 0;
	}

	virtual std::string ToString(const T& arg) const {
		if constexpr (templ::is_printable<T, std::stringstream>::value) {
			std::stringstream ss;
			ss << arg;
			return ss.str();
		}
		else {
			throw InvalidCallException("This type cannot be converted to string.");
		}
	}
private:
	template <class Head, class... Functions>
	void RegisterFunctions(Head head, Functions... functions) {
		RegisterFunction(head);
		RegisterFunctions(functions...);
	}
	void RegisterFunctions() {}

	template <class SourceT>
	void RegisterFunction(T(*function)(SourceT)) {
		auto convfunc = [function](const void* src, void* dst)
		{
			*reinterpret_cast<T*>(dst) = function(*reinterpret_cast<const std::remove_reference_t<SourceT>*>(src));
		};
		m_converters.insert({
			typeid(SourceT),
			convfunc
		});
	}
private:
	std::unordered_map<std::type_index, std::function<void(const void*, void*)>> m_converters;
};

template <>
class PortConverterCollection<void> {
public:
	using Functor = void(*)(const void*, void*);
	Functor operator[](std::type_index type) const {
		throw OutOfRangeException("Cannot find a converter for type.", type.name());
	}
	bool CanConvert(std::type_index type) const {
		return false;
	}
};


template <class T, bool Readable> 
class DefaultPortConverter : public PortConverterCollection<T> 
{};

template <class T>
class DefaultPortConverter<T, true> : public PortConverterCollection<T> {
public:
	DefaultPortConverter() : PortConverterCollection<T>(&FromString) 
	{}
protected:
	static T FromString(const std::string& str) {
		T val;
		std::stringstream ss(str);
		ss >> val;
		if (!ss.fail()) {
			throw InvalidArgumentException("String could not be converted to type.");
		}
		return val;
	}
};


/// <summary>
/// Converts types when passed between output->input ports.
/// <para> To implement converters for a certain type, specialize
///		this class for the type. The specialization must have an operator[]
///		taking an std::type_index, and returning a (const void*, void) functor.
///		The functor should convert the source type (defined by the type_index param)
///		to the destination type (defined by the specialization). </para>
/// <para> If the conversion is not possible or not implemented, the operator[]
///		method should throw an std::out_of_range. </para>
/// </summary>
template <class T>
class PortConverter : public DefaultPortConverter<T, templ::is_readable<T>::value> 
{};



/// <summary>
/// <para> Output port of a node. </para>
/// <para>
/// Output ports are attached to nodes. They can be linked to
/// input ports. A node can activate them with data, and that data
/// is forwarded to connected input ports. An output port can be linked
/// to multiple input ports at the same time.
/// </para>
/// </summary>
class OutputPortBase : public ISerializableOutputPort {
public:
	using LinkIterator = std::set<InputPortBase*>::iterator;
	using ConstLinkIterator = std::set<InputPortBase*>::const_iterator;
public:
	OutputPortBase();
	~OutputPortBase();

	/// <summary> Get typeid of underlying data. </summary>
	virtual std::type_index GetType() const = 0;

	/// <summary> Link to an input port. </summary>
	/// <returns> True if succesfully linked. Make sures types are compatible. </returns>
	/// <exception cref="InvalidArgumentException"> In case the other port is already linked or types are incompatible. </exception>
	void Link(InputPortBase* destination);

	/// <summary> Remove link between this and the other end. </summary>
	/// <param param="other"> The port to unlink from this. </param>
	virtual void Unlink(InputPortBase* other);

	/// <summary> Unlink all ports from this. </summary>
	virtual void UnlinkAll();

	// Iterate over links.
	LinkIterator begin();
	LinkIterator end();
	ConstLinkIterator begin() const;
	ConstLinkIterator end() const;
	ConstLinkIterator cbegin() const;
	ConstLinkIterator cend() const;
protected:
	std::set<InputPortBase*> links;
};



/// <summary>
/// <para> Input port of a Node. </para>
/// <para> 
/// Input ports are attached to a node. An input port can also be linked
/// to an output port, from where it receives the data.
/// </para>
/// </summary>
class InputPortBase : public ISerializableInputPort {
	friend class OutputPortBase;
	friend class OutputPort<Any>;
public:
	InputPortBase();
	~InputPortBase();

	/// <summary> Clear currently stored data. </summary>
	virtual void Clear() = 0;

	/// <summary> Get weather any valid data is set. </summary>
	virtual bool IsSet() const override = 0;

	/// <summary> Get typeid of underlying data. </summary>
	virtual std::type_index GetType() const = 0;
	/// <summary> Get if can convert from certain type. </summary>
	virtual bool IsCompatible(std::type_index type) const = 0;

	/// <summary> Link this port to an output port. </summary>
	/// <returns> True if succesfully linked. Make sures types are compatible. </returns>
	/// <exception cref="InvalidStateException"> In case the port is already linked. </exception>
	/// <exception cref="InvalidArgumentException">  If types are incompatible. </exception>
	void Link(OutputPortBase* source);

	/// <summary> Remove link between this and the other end. </summary>
	void Unlink();

	/// <summary> Add observer node.
	/// Observers are notified when new data is set. </summary>
	virtual void AddObserver(NodeBase* observer) final;
	/// <summary> Remove observer. </summary>
	virtual void RemoveObserver(NodeBase* observer) final;

	/// <summary> Get which output port it is linked to. </summary>
	/// <returns> The other end. Null if not linked. </returns>
	OutputPortBase* GetLink() const override;

	/// <summary> Set type that is to be converted automatically. </summary>
	template <class U>
	void SetConvert(const U& u);

	/// <summary> Converts underlying data to string using it's &lt;&lt; operator </summary>
	/// <exception cref="InvalidCallException"> If no ostream operator available. </exception> 
	virtual std::string ToString() const override = 0;
protected:
	OutputPortBase* link;
	void NotifyAll();
	virtual void SetConvert(const void* object, std::type_index type) = 0;
private:
	// should only be called by an output port when it's ready with building up the linkage
	// this function only sets internal state of the inputport to represent the link set up by outputport
	void SetLinkState(OutputPortBase* link);

	std::set<NodeBase*> observers;
};



/// <summary>
/// <para> Specialization of InputPortBase for various types of data. </para>
/// <para> Different types can be set as template parameter. Generally, it's enough to
/// just use this template, but it may be necessary to specialize this template
/// for certain data types to improve efficiency or change behaviour. </para>
/// </summary>
template <class T, class ConverterT = PortConverter<T>>
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

	std::string ToString() const override {
		return converter.ToString(data);
	}

	virtual bool IsCompatible(std::type_index type) const override;
protected:
	virtual void SetConvert(const void* object, std::type_index type) override;
private:
	bool isSet;
	T data;
	ConverterT converter;
};


template <class T, class ConverterT>
void InputPort<T, ConverterT>::SetConvert(const void* object, std::type_index type) {
	isSet = true;
	if (type == typeid(T)) {
		data = *reinterpret_cast<const T*>(object);
	}
	else {
		converter[type](object, &data);
	}
}


template <class T, class ConverterT>
bool InputPort<T, ConverterT>::IsCompatible(std::type_index type) const {
	if (type == typeid(T)) {
		return true;
	}
	else {
		return converter.CanConvert(type);
	}
}


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



//------------------------------------------------------------------------------
// Specialization for void type ports
//------------------------------------------------------------------------------
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

	bool IsCompatible(std::type_index type) const override {
		return true;
	}

	std::string ToString() const override {
		throw InvalidCallException("Void ports cannot be converted to a string.");
	}
protected:
	void SetConvert(const void* object, std::type_index type) override {
		isSet = true;
		// conversion does nothing
	}
private:
	bool isSet;
};



//------------------------------------------------------------------------------
// Specialization for void type ports.
//------------------------------------------------------------------------------
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


class NullPortConverter {};

//------------------------------------------------------------------------------
// Template specialization for any-type ports.
//------------------------------------------------------------------------------
template <class ConverterT>
class InputPort<Any, ConverterT> : public InputPortBase {
public:
	InputPort() = default;

	/// Set data as input.
	/// As a template function, it accepts any type of data.
	/// \return Returns true, always. Should return false if type constraints \
	/// don't allow insertion of data, but type constraints are not implemented yet.
	template <class U>
	void Set(U&& data) {
		this->data = Any(std::forward<U>(data));
		NotifyAll();
	}

	void Set(const Any& in) {
		this->data = in;
		NotifyAll();
	}

	/// Set void data.
	/// May be called by void type ports.
	bool Set() {
		Clear();
		NotifyAll();
		return true;
	}

	const Any& Get() const {
		return data;
	}

	void Clear() override {
		data = {};
	}

	bool IsSet() const override {
		return (bool)data;
	}

	std::type_index GetType() const override {
		return typeid(Any);
	}

	bool IsCompatible(std::type_index type) const override {
		return type != typeid(void);
	}

	std::string ToString() const override {
		throw InvalidCallException("Any ports cannot be converted to string.");
	}
protected:
	void SetConvert(const void* object, std::type_index type) override {
		throw InvalidCallException("Any-type ports cannot convert anything.");
	}
private:
	Any data;
};


//------------------------------------------------------------------------------
// Template specialization for any-type ports.
//------------------------------------------------------------------------------
template <>
class OutputPort<Any> : public OutputPortBase {
public:
	OutputPort() : currentType(typeid(Any)) {}

	/// Set data on this output port.
	/// Keep in mind that mismatching input will NOT be forwarded to input ports
	/// linked to this output port. For example, a recieved "float" type won't
	/// be sent to a linked "int" type input port.
	/// \param data A special object which contains the real data.
	void Set(const Any& data) {
		for (auto& v : links) {
			if (v->GetType() != typeid(Any)) {
				v->SetConvert(data.Raw(), data.Type());
			}
			else {
				static_cast<InputPort<Any>*>(v)->Set(data);
			}
		}
	}

	/// Get type of this port.
	/// \return Always typeid(Any).
	std::type_index GetType() const override {
		return typeid(Any);
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
		if (v->GetType() == GetType()) {
			static_cast<InputPort<T>*>(v)->Set(data);
		}
		else if (v->GetType() == typeid(Any)) {
			static_cast<InputPort<Any>*>(v)->Set(data);
		}
		else {
			v->SetConvert(data);
		}
	}
}



//------------------------------------------------------------------------------
// Misc methods
//------------------------------------------------------------------------------
template <class U>
void InputPortBase::SetConvert(const U& u) {
	if (GetType() != typeid(Any)) {
		SetConvert(reinterpret_cast<const void*>(&u), typeid(U));
	}
	else {
		static_cast<InputPort<Any>*>(this)->Set(u);
	}
}


//------------------------------------------------------------------------------
// Explicit instantiations
//------------------------------------------------------------------------------
extern template class InputPort<Any, NullPortConverter>;
extern template class OutputPort<Any>;

} // namespace inl



// Now we include the port converters so that they are available if someone includes this files.
// It needs to be included down here though cause it also includes this file causing a cross-reference.
// For the cross-ref to be okay, the definition of ports must be available at the point the converters are included.
#include "PortConverters.hpp"