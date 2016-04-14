#pragma once

// inheritence by dominance
// derived classes of ???PortConfig inherit methods from multiple sources or smth like that
#ifdef _MSC_VER
#pragma warning(disable: 4250)
#endif

#include "Port.hpp"

#include <cstddef>
#include <type_traits>
#include <string>
#include <initializer_list>
#include <vector>
#include <typeinfo>
#include <typeindex>



namespace exc {


class InputPortBase;


/// <summary>
/// <para> Base class for nodes. </para>
/// <para>
/// Nodes are basic unit of data manipulation. They are connected into
/// a network, which has some sources and sinks. Such networks help visually
/// implement data processing and logic, where each node transform the data
/// in some way.
/// </para>
/// <para>
///	A single node can have multiple input port and multiple output ports.
/// Each port is typed, that is, they may be float, int, or something else.
/// An output port of a node can be connected to the input port of another,
/// but only if their types are compatible. This way, the network is created.
/// </para>
/// <para>
/// To create your own nodes, override methods of this interface. Nodes may
/// be registered to the node factory if they have default constructor and
/// implement the required static methods. To quickly add ports, inherit from
/// InputPortConfig and/or OutputPortConfig.
/// </para>
/// </summary>
class NodeBase {
public:
	virtual ~NodeBase() = default;

	/// <summary> Returns the number of input ports. </summary>
	virtual size_t GetNumInputs() const = 0;
	/// <summary> Returns the number of output ports. </summary>
	virtual size_t GetNumOutputs() const = 0;

	/// <summary> Get pointer to the indexth input port. </summary>
	virtual InputPortBase* GetInput(size_t index) = 0;
	/// <summary> Get pointer to the indexth output port. </summary>
	virtual OutputPortBase* GetOutput(size_t index) = 0;

	/// <sumary> Get pointer to the indexth input port. </summary>
	virtual const InputPortBase* GetInput(size_t index) const = 0;
	/// <summary> Get pointer to the indexth output port. </summary>
	virtual const OutputPortBase* GetOutput(size_t index) const = 0;

	/// <summary> Read and process input ports and activate output. </summary>
	virtual void Update() = 0;
	/// <summary> Called by the input ports of the node to notify new data. </summary>
	virtual void Notify(InputPortBase* sender) = 0;
};



/// <summary>
/// <para> Generate input ports of specified types. </para>
/// <para>
///	Specify port type on the template argument list. InputPortConfig
/// then contains the specified port types, which may be accessed by
/// GetInput&lt;Index&gt;(). Inhreit from this class to add input nodes
/// and implement required input-related interfaces.
/// </para>
/// </summary>
template <class... Types>
class InputPortConfig : virtual public NodeBase {
public:
	/// <summary> Initialize input ports. </sumamry>
	InputPortConfig() {
		InitTable<sizeof...(Types)-1>();
	}
	/// <summary> Initiaize input ports from given arguments. </summary>
	template <class... Args>
	explicit InputPortConfig(Args&&... args)
		: ports(std::forward<Args>(args)...)
	{
		InitTable<sizeof...(Types)-1>();
	}

	/// <summary> Get reference to typed input port by its index. </summary>
	template <size_t Index>
	auto& GetInput() {
		return std::get<Index>(ports);
	}

	/// <summary> Get reference to typed input port by its index. </summary>
	template <size_t Index>
	const auto& GetInput() const {
		return std::get<Index>(ports);
	}

	/// <summary> Get pointer to the indexth input port. </summary>
	InputPortBase* GetInput(size_t index) override {
		if (index >= sizeof...(Types))
			return nullptr;
		else
			return portTable[index];
	}

	/// <summary> Get pointer to the indexth input port. </summary>
	const InputPortBase* GetInput(size_t index) const override {
		if (index >= sizeof...(Types))
			return nullptr;
		else
			return portTable[index];
	}

	/// <summary> Returns the number of inputs. </summary>
	size_t GetNumInputs() const override {
		return sizeof...(Types);
	}

	/// <summary> Returns the number of inputs. </summary>
	static constexpr size_t Info_GetNumInputs() {
		return sizeof...(Types);
	}

	/// <summary> Get typeid's of input types. </summary>
	static const std::vector<std::type_index>& Info_GetInputTypes() {
		static std::vector<std::type_index> inputTypes = {
			typeid(Types)...
		};
		return inputTypes;
	}
private:
	// Initialize a pointer table to get inputs with dynamic indices.
	template <size_t Index, typename std::enable_if<Index != 0, void>::type* = nullptr>
	void InitTable() {
		portTable[Index] = &std::get<Index>(ports);
		InitTable<Index - 1>();
	}
	template <size_t Index, typename std::enable_if<Index == 0, void>::type* = nullptr>
	void InitTable() {
		portTable[0] = &std::get<0>(ports);
	}

	std::tuple<InputPort<Types>...> ports;
	InputPortBase* portTable[sizeof...(Types)];
};


// Template specialization for empty input port config.
template <>
class InputPortConfig<> : virtual public NodeBase {
public:

	size_t GetNumInputs() const override {
		return 0;
	}

	/// <summary> Get pointer to the indexth input port. </summary>
	InputPortBase* GetInput(size_t index) override {
		return nullptr;
	}

	/// <summary> Get pointer to the indexth input port. </summary>
	const InputPortBase* GetInput(size_t index) const override {
		return nullptr;
	}

	/// <summary> Returns the number of inputs. </summary>
	static constexpr size_t Info_GetNumInputs() {
		return 0;
	}

	/// <summary> Get typeid's of input types. </summary>
	static const std::vector<std::type_index>& Info_GetInputTypes() {
		static std::vector<std::type_index> inputTypes{};
		return inputTypes;
	}
};



/// <summary>
/// <para> Generate output ports of specified types. </para>
/// <para>
///	Specify port type on the template argument list. OutputPortConfig
/// then contains the specified port types, which may be accessed by
/// GetOutput&lt;Index&gt;(). Inhreit from this class to add output nodes
/// and implement required output-related interfaces.
/// </para>
/// </summary>
template <class... Types>
class OutputPortConfig : virtual public NodeBase {
public:
	/// <summary> Initialize output ports. </sumamry>
	OutputPortConfig() {
		InitTable<sizeof...(Types)-1>();
	}
	/// <summary> Initiaize output ports from given arguments. </summary>
	template <class... Args>
	explicit OutputPortConfig(Args&&... args)
		: ports(std::forward<Args>(args)...)
	{
		InitTable<sizeof...(Types)-1>();
	}

	/// <summary> Get reference to typed output port by its index. </summary>
	template <size_t Index>
	auto& GetOutput() {
		return std::get<Index>(ports);
	}

	/// <summary> Get reference to typed output port by its index. </summary>
	template <size_t Index>
	const auto& GetOutput() const {
		return std::get<Index>(ports);
	}

	/// <summary> Get pointer to the indexth output port. </summary>
	OutputPortBase* GetOutput(size_t index) override {
		if (index >= sizeof...(Types))
			return nullptr;
		else
			return portTable[index];
	}

	/// <summary> Get pointer to the indexth output port. </summary>
	const OutputPortBase* GetOutput(size_t index) const override {
		if (index >= sizeof...(Types))
			return nullptr;
		else
			return portTable[index];
	}
	/// <summary> Returns the number of outputs. </summary>
	size_t GetNumOutputs() const override {
		return sizeof...(Types);
	}

	/// <summary> Get typeid's of output types. </summary>
	static const std::vector<std::type_index>& Info_GetOutputTypes() {
		static std::vector<std::type_index> outputTypes = {
			typeid(Types)...
		};
		return outputTypes;
	}

	/// <summary> Returns the number of outputs. </summary>
	static constexpr size_t Info_GetNumOutputs() {
		return sizeof...(Types);
	}
private:
	template <size_t Index, typename std::enable_if<Index != 0, void>::type* = nullptr>
	void InitTable() {
		portTable[Index] = &std::get<Index>(ports);
		InitTable<Index - 1>();
	}
	template <size_t Index, typename std::enable_if<Index == 0, void>::type* = nullptr>
	void InitTable() {
		portTable[0] = &std::get<0>(ports);
	}

	std::tuple<OutputPort<Types>...> ports;
	OutputPortBase* portTable[sizeof...(Types)];
};


// Template specialization for empty output port config.
template <>
class OutputPortConfig<> : virtual public NodeBase {
public:
	/// <summary> Returns the number of outputs. </summary>
	size_t GetNumOutputs() const override {
		return 0;
	}

	/// <summary> Get pointer to the indexth output port. </summary>
	OutputPortBase* GetOutput(size_t index) override {
		return nullptr;
	}

	/// <summary> Get pointer to the indexth output port. </summary>
	const OutputPortBase* GetOutput(size_t index) const override {
		return nullptr;
	}

	/// <summary> Get typeid's of output types. </summary>
	static const std::vector<std::type_index>& Info_GetOutputTypes() {
		static std::vector<std::type_index> outputTypes{};
		return outputTypes;
	}

	/// <summary> Returns the number of outputs. </summary>
	static constexpr size_t Info_GetNumOutputs() {
		return 0;
	}
};


} // namespace exc