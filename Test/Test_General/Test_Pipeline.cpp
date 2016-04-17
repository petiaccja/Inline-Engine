#include "Test.hpp"

#include <GraphicsEngine_LL/GraphicsNodeFactory.hpp>
#include <GraphicsEngine_LL/Pipeline.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>
#include <BaseLibrary/Graph_All.hpp>

#include <iostream>
#include <fstream>

using namespace std;
using namespace inl::gxeng;
using namespace exc;


//------------------------------------------------------------------------------
// Node classes
//------------------------------------------------------------------------------


class TestNode : public InputPortConfig<int, int>, public OutputPortConfig<int> {
public:
	static std::string Info_GetName() { return "TestNode"; }
	static std::vector<std::string> Info_GetInputNames() { return{ "in" }; }
	static std::vector<std::string> Info_GetOutputNames() { return{ "out" }; }

	virtual void Update() override {
		auto input = GetInput<0>();
		auto output = GetOutput<0>();
		auto inputValue = input.Get();
		output.Set(inputValue + 1);
		cout << "id = " << id << ", " << "value = " << inputValue << endl;
	}
	virtual void Notify(InputPortBase*) override {

	}

	mutable int id;
};

class TestGraphicsNode : public TestNode, public GraphicsNode {
public:
	static std::string Info_GetName() { return "TestGraphicsNode"; }
	static std::vector<std::string> Info_GetInputNames() { return{ "in" }; }
	static std::vector<std::string> Info_GetOutputNames() { return{ "out" }; }

	void Update() override {
		auto input = GetInput<0>();
		auto output = GetOutput<0>();
		auto inputValue = input.Get();
		output.Set(inputValue + 1);
		cout << "id = " << id << ", " << "value = " << inputValue << endl;
	}
	void Notify(InputPortBase*) override {

	}
	ExecutionResult DoNothing(ExecutionContext) {
		// do nothing
		return ExecutionResult{};
	}
	Task GetTask() override {
		return Task{ {
				ElementaryTask([this](ExecutionContext ctx) { return DoNothing(ctx); }),
				ElementaryTask([this](ExecutionContext ctx) { return DoNothing(ctx); }),
				ElementaryTask([this](ExecutionContext ctx) { return DoNothing(ctx); }),
				ElementaryTask([this](ExecutionContext ctx) { return DoNothing(ctx); }),
		} };
	}
};


//------------------------------------------------------------------------------
// Helpers
//------------------------------------------------------------------------------


void Graphviz(const lemon::ListDigraph& graph, const lemon::ListDigraph::NodeMap<std::string>& labels, std::ostream& output) {
	output << "digraph G {\n";
	for (lemon::ListDigraph::ArcIt arc(graph); arc != lemon::INVALID; ++arc) {
		output << labels[graph.source(arc)] << " -> " << labels[graph.target(arc)] << ";\n";
	}
	output << "}" << endl;
}



//------------------------------------------------------------------------------
// Test class
//------------------------------------------------------------------------------


class TestPipeline : public AutoRegisterTest<TestPipeline> {
public:
	TestPipeline() {}

	static std::string Name() {
		return "Pipeline";
	}
	virtual int Run() override;
private:
	static int a;
};


//------------------------------------------------------------------------------
// Test definition
//------------------------------------------------------------------------------


int TestPipeline::Run() {
	cout << "Creating factory..." << endl;

	GraphicsNodeFactory factory;
	factory.RegisterNodeClass<TestNode>("");
	factory.RegisterNodeClass<TestGraphicsNode>("");

	cout << "Creating pipeline..." << endl;

	Pipeline pipeline;
	pipeline.SetFactory(&factory);

	try {
		// add nodes to pipeline
		for (int i = 0; i < 5; i++) {
			auto it = pipeline.AddNode("TestNode");
			dynamic_cast<const TestNode&>(*it).id = i;
		}
		{
			auto itSp = pipeline.AddNode("TestGraphicsNode");
			dynamic_cast<const TestGraphicsNode&>(*itSp).id = 100;
			itSp = pipeline.AddNode("TestGraphicsNode");
			dynamic_cast<const TestGraphicsNode&>(*itSp).id = 101;
		}

		// sort nodes by id
		std::vector<Pipeline::NodeIterator> nodes;
		for (auto it = pipeline.Begin(); it != pipeline.End(); ++it) {
			nodes.push_back(it);
		}
		std::sort(nodes.begin(), nodes.end(), [](auto lhs, auto rhs) {
			return dynamic_cast<const TestNode&>(*lhs).id < dynamic_cast<const TestNode&>(*rhs).id;
		});
		for (auto& node : nodes) {
			//cout << dynamic_cast<const TestNode&>(*node).id << endl;
		}

		// add a few links
		pipeline.AddLink(nodes[0], 0, nodes[1], 0);
		pipeline.AddLink(nodes[0], 0, nodes[2], 0);
		pipeline.AddLink(nodes[0], 0, nodes[2], 1);
		pipeline.AddLink(nodes[1], 0, nodes[5], 0);
		pipeline.AddLink(nodes[2], 0, nodes[3], 0);
		pipeline.AddLink(nodes[5], 0, nodes[4], 0);
		pipeline.AddLink(nodes[2], 0, nodes[5], 1);
		pipeline.AddLink(nodes[3], 0, nodes[4], 1);
		pipeline.AddLink(nodes[4], 0, nodes[6], 0);

		// draw dependency graph
		ofstream file("depgraph.dot");
		lemon::ListDigraph::NodeMap<std::string> labelMap(pipeline.GetDependencyGraph());
		for (lemon::ListDigraph::NodeIt node(pipeline.GetDependencyGraph()); node != lemon::INVALID; ++node) {
			TestNode* testNode = dynamic_cast<TestNode*>(pipeline.GetNodeMap()[node]);
			TestGraphicsNode* testGraphicsNode = dynamic_cast<TestGraphicsNode*>(pipeline.GetNodeMap()[node]);
			stringstream ss;
			ss << "node_";
			if (testNode) {
				ss << testNode->id;
			}
			else {
				ss << testNode->id;
			}
			labelMap[node] = ss.str();
		}
		Graphviz(pipeline.GetDependencyGraph(), labelMap, file);
		file.close();

		// create task graph
		pipeline.CalculateTaskGraph_Dbg();

		// draw task graph
		file.open("taskgraph.dot");
		lemon::ListDigraph::NodeMap<std::string> taskLabel(pipeline.GetTaskGraph());
		int taskIndex = 0;
		for (lemon::ListDigraph::NodeIt node(pipeline.GetTaskGraph()); node != lemon::INVALID; ++node) {
			bool isDummyTask = !(bool)pipeline.GetTaskMap()[node];
			stringstream ss;
			ss << (isDummyTask ? "dummy_" : "") << "task_" << taskIndex++;
			taskLabel[node] = ss.str();
		}
		Graphviz(pipeline.GetTaskGraph(), taskLabel, file);
		file.close();
	}
	catch (std::exception& ex) {
		cout << "Failed to create pipeline: " << ex.what() << endl;
	}

	return 0;
}