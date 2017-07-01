#pragma once


#include "../BaseLibrary/Graph/Node.hpp"
#include "NodeContext.hpp"


#ifdef _MSC_VER // disable lemon warnings 
#pragma warning(push) 
#pragma warning(disable: 4267) 
#endif 

#include <lemon/list_graph.h> 

#ifdef _MSC_VER 
#pragma warning(pop) 
#endif 



namespace inl::gxeng {



class EngineContext;
class SetupContext;
class RenderContext;



/// <summary>
/// A graphics task is responsible for both CPU-side and GPU-side data processing in the 
/// node-based pipeline graph.
/// </summary>
/// <remarks>
/// <para> Graphics tasks won't work alone, they must be part of a <see cref="GraphicsNode"/>, which
/// contains a directed acyclic graph of graphics tasks. Graphics nodes should create their own
/// graphics task graphs at runtime, based on conditions like number of CPU threads. This scheme
/// facilitates the parallelization of the work done by a single graphics node.
/// </para>
/// <para> Rendering the frame is done in two phases.
///	The first phase (<see cref="GraphicsTask::Setup"/>) allows the node to create the resources
/// and resource view to render the current frame. By this time, all the node input ports should be 
/// considered defined. Resource views are tied to the graphics task that created them, 
/// and passing and using them in another graphics task will be blocked by the engine. No restrictions
/// apply to the resources themselves. Resources and view can be stored and held over multiple frames.
/// In the second phase (<see cref="GraphicsTask::Execute"/>) the node gets limited access to creation
/// of resource that are solely for this one frame, and shouldn't be stored. Besides, nodes can get access
/// to a command list which they can populate with the actual GPU rendering commands.
/// </para>
/// </remarks>
/// <seealso cref="GraphicsNode"/>
class GraphicsTask {
public:
	virtual void Setup(SetupContext& context) = 0;
	virtual void Execute(RenderContext& context) = 0;
};


/// <summary>
/// Nodes have several input and output ports, which, linked together, form the graphical representation
/// of the rendering pipeline that runs each frame to render the scene.
/// </summary>
/// <remarks>
/// Internally, each node is made up of a directed acyclic graph of <see cref="GraphicsTask"/> objects.
/// During inizialization, the node should build up its own graph. Nodes merely act as an input-output
/// port collection, that define the data flow between nodes. Node also represent a transformation
/// that is done on the input to create the output. The actual transformation, however, is done by the
/// graphics tasks in the nodes graph. This is useful when the node wants to parallelize its work,
/// in which case it create parallel graphics tasks within itself.
/// </remarks>
class GraphicsNode : virtual public NodeBase {
public:
	GraphicsNode();

	virtual void Initialize(EngineContext& context) = 0;
	virtual void Reset() = 0;

	const lemon::ListDigraph& GetTaskGraph() const;
	const lemon::ListDigraph::NodeMap<GraphicsTask*>& GetTaskGraphMapping() const;

protected:
	void SetTaskSingle(GraphicsTask* task);

	template <class Iter>
	void SetTaskParallel(Iter first, Iter last);

	template <class Iter>
	void SetTaskSequential(Iter first, Iter last);

	void SetTaskGraph(const lemon::ListDigraph& nodes, const lemon::ListDigraph::NodeMap<GraphicsTask*>& map);

private:
	lemon::ListDigraph m_taskNodes;
	lemon::ListDigraph::NodeMap<GraphicsTask*> m_taskMap;
};



template <class Iter>
void GraphicsNode::SetTaskParallel(Iter first, Iter last) {
	static_assert(std::is_convertible<decltype(*first), GraphicsTask*>::value, "Iterators must point to objects of type GraphicsTask*.");

	m_taskNodes.clear();

	for (Iter it = first; it != last; ++it) {
		lemon::ListDigraph::Node node = m_taskNodes.addNode();
		m_taskMap[node] = *it;
	}
}


template <class Iter>
void GraphicsNode::SetTaskSequential(Iter first, Iter last) {
	static_assert(std::is_convertible<decltype(*first), GraphicsTask*>::value, "Iterators must point to objects of type GraphicsTask*.");

	m_taskNodes.clear();

	lemon::ListDigraph::Node prevNode = lemon::INVALID;
	for (Iter it = first; it != last; ++it) {
		lemon::ListDigraph::Node node = m_taskNodes.addNode();
		m_taskMap[node] = *it;
		if (prevNode != lemon::INVALID) {
			m_taskNodes.addArc(prevNode, node);
		}
		prevNode = node;
	}
}


} // namespace inlgxeng::gxeng