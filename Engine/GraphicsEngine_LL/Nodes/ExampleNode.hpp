#pragma once


#include <GraphicsEngine_LL/GraphicsNode.hpp>
#include "../PipelineTypes.hpp"
#include "../ResourceView.hpp"
#include <GraphicsEngine_LL/GraphicsCommandList.hpp>


namespace inl::gxeng::nodes {

// To create a node, you generally have to inherit from NodeBase, and you are all set.
// To enable graphics engine specific things, such as creating resources or access to drawing commands, 
// you must inherit from GraphicsNode instead.
//
// To declare input ports and output, inherit from the InputPortConfig and OutputPortConfig
// helper classes. This is not mandatory, you can roll you own implementation by implementing
// NodeBase's interface manually. Refer to InputPort<T> and OutputPort<T>.
//
// You should use virtual inheritence because of the way these helpers are implemented.
class ExampleNode 
	: virtual public GraphicsNode,
	// Comment the names of the input, to let others know:
	// Input HDR color texture | tone-mapping key
	virtual public InputPortConfig<Texture2D, float>,
	// Same for outputs:
	// Blurred output
	virtual public OutputPortConfig<Texture2D>
{
private:
	// [Graphics nodes only]
	// Graphics nodes must implement sub-tasks, which means that the actual work is not done
	// by the node-class, but by a graph of sub-tasks. Only sub-tasks have access to rendering
	// commands. The rationale behind this is that a graphics node can contain a directed acyclic graph
	// of sub-tasks, thus, it might parallelize its work by creating multiple sub-tasks.
	//
	// A sub-task must implement the GraphicsTask interface. If you find it simpler, your
	// graphics node class can also implement the interface, and be the sub-task itself.
	//
	// [THREADING] Sub-tasks of even the same may be run in parallel. Make sure to lock state you
	// share via the parent node. Do not hold locks for a long time, as it will disrupt task scheduling.
	class ExampleTask : public GraphicsTask {
	public:
		ExampleTask(ExampleNode* parent) : m_parent(parent) {}

		// PHASE I.
		// In the setup function, you should create most resources that you will use in this frame.
		// You can base your allocations on the inputs, which won't change throughout the rest of the frame.
		// Set your outputs in this phase as well.
		void Setup(SetupContext& context) override {
			// Read inputs.
			Texture2D sourceTexture = m_parent->GetInput<0>().Get();
			float key = m_parent->GetInput<1>().Get();

			// Validate cached resources, and create new if necessary.
			if (!m_outputTexture.HasObject()
				|| sourceTexture.GetWidth() != m_outputTexture.GetWidth()
				|| sourceTexture.GetHeight() != m_outputTexture.GetHeight())
			{
				m_outputTexture = context.CreateRenderTarget2D(sourceTexture.GetWidth(),
															   sourceTexture.GetHeight(),
															   sourceTexture.GetFormat());
			}
			// [IMPORTANT] The RESOURCE VIEWS you create here must NOT be SHARED with other graphics sub-tasks.
			// You can still share resources via the parent node.

			// Dissatisfied with inputs, any problems occured?
			if (key < 0.0f) {
				// You are allowed to throw an exception with a meaningful message as to what went wrong.
				// [NOT YET IMPLEMENTED - you should still throw, nonetheless] The scheduler will display 
				// a graph of the pipeline, highlighting the wrong node, and displaying the error message.
				throw InvalidArgumentException("Tone-mapping key must not be negative!");
			}

			// When frame's initialization is done, set outputs.
			// [IMPORTANT] Do NOT PASS resource views through output ports in any way! This would
			// fool the scheduler resource tracking system, thus it will detect the error and tell you
			// you are in the wrong.
			m_parent->GetOutput<0>().Set(m_outputTexture);
		}

		// PHASE II.
		// In the execute function, you gain full access to rendering commands. Your resource creation
		// is restricted, but you should be able to issue all your rendering commands using the resources
		// from the setup phase, and the ones you create here.
		// [IMPORTANT] Do NOT CHANGE the value of your OUTPUT ports, as this will likely invalidate
		// the next nodes' setup work.
		// You are allowed to freely read (and modify, though there's no reason for that) your input ports.
		void Execute(RenderContext& context) override {
			// First, get a command list of your choice. This is optional, you may not want any.
			// Do not copy or store this command list, keep only a reference or a pointer throughout execute.
			// Further calls to different types (AsCompute and AsCopy in this case) will fail with an exception.
			// You might call the same type (AsGraphics, in this case) multiple times, but it will be the same list.
			GraphicsCommandList& list = context.AsGraphics();

			// Issue your rendering commands.
			// Feel free to use the render context to create views, resources, and similar.

			// You do not need anything to finalize your command list. Just leave this function.
		}

		// Please refer to node's reset.
		void Reset() {
			m_parent->GetInput<0>().Clear(); // Be sure to clear inputs that hold resource as well.
			m_outputTexture = Texture2D();

			// Don't worry, you will recreate all you need in the next setup.
		}
	private:
		ExampleNode* m_parent; // store a reference to the parent Node to access its ports
		Texture2D m_outputTexture;
	};

public:
	// This function is part of NodeBase. For general nodes, you should read the input
	// and calculate the outputs in this function. For graphics nodes, it is not used.
	void Update() override {}


	// This function is part of NodeBase. The graphics pipeline does not use it, you should leave
	// it blank, and not rely on it.
	void Notify(InputPortBase* sender) override {}
	

	// Called just after the pipeline is created. This is the place where the Node has access
	// to engine-specific parameters, such as number of CPUs. Create your sub-tasks here.
	void Initialize(EngineContext& context) override {
		// You might create parallel graphs if there are more CPU cores.
		// For now, we will create a single sub-task:
		m_subtask = std::make_unique<ExampleTask>(this);
		SetTaskSingle(m_subtask.get());
	}


	// The reset function is called when the pipeline nodes have to let go of all
	// resources they are holding. Its not a frequent thing, but you must release
	// all resources and resource views, even from sub-tasks, when this is called.
	void Reset() override {
		m_subtask->Reset();
	}
private:
	std::unique_ptr<ExampleTask> m_subtask;
};






} // namespace inl::gxeng::nodes