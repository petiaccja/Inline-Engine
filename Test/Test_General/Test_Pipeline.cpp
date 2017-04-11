#include "Test.hpp"

#include <GraphicsEngine_LL/GraphicsNodeFactory.hpp>
#include <GraphicsEngine_LL/Pipeline.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>
#include <BaseLibrary/Graph_All.hpp>

#include <iostream>
#include <fstream>
#include <GraphicsEngine_LL/Scheduler.hpp>

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
		auto input1 = GetInput<0>();
		auto input2 = GetInput<1>();
		auto output = GetOutput<0>();
		auto inputValue = input1.Get() + input2.Get() + 1;
		output.Set(inputValue);
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
		auto input1 = GetInput<0>();
		auto input2 = GetInput<1>();
		value = input1.Get() + input2.Get();
		cout << "id = " << id << ", " << "value = " << value << endl;
		GetOutput<0>().Set(value);
	}

	void Notify(InputPortBase*) override {

	}

	// OBSOLETE

	//void InitGraphics(const GraphicsContext&) override {}

	//ExecutionResult DoSomething(ExecutionContext) {
	//	++value;

	//	auto input1 = GetInput<0>();
	//	auto input2 = GetInput<1>();
	//	int sum = input1.Get() + input2.Get() + value;
	//	GetOutput<0>().Set(sum);
	//	cout << "id = " << id << ", " << "value = " << sum << endl;
	//	return ExecutionResult{};
	//}

	//Task GetTask() override {
	//	return Task{ {
	//			ElementaryTask([this](ExecutionContext ctx) { return DoSomething(ctx); }),
	//			ElementaryTask([this](ExecutionContext ctx) { return DoSomething(ctx); }),
	//			ElementaryTask([this](ExecutionContext ctx) { return DoSomething(ctx); }),
	//			ElementaryTask([this](ExecutionContext ctx) { return DoSomething(ctx); }),
	//	} };
	//}
private:
	int value = 0;
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
	//factory.RegisterNodeClass<TestNode>("");
	//factory.RegisterNodeClass<TestGraphicsNode>("");

	cout << "Creating pipeline..." << endl;

	Pipeline pipeline;
	//Scheduler scheduler;
	/*
	try {		
		// create a few nodes
		TestNode node0, node1, node2, node3, node4;
		TestGraphicsNode gnode0, gnode1;

		node0.id = 0;
		node1.id = 1;
		node2.id = 2;
		node3.id = 3;
		node4.id = 4;
		gnode0.id = 100;
		gnode1.id = 101;

		node0.GetInput<0>().Set(0);
		node0.GetInput<1>().Set(0);
		node1.GetInput<0>().Set(0);
		node1.GetInput<1>().Set(0);
		node2.GetInput<0>().Set(0);
		node2.GetInput<1>().Set(0);
		node3.GetInput<0>().Set(0);
		node3.GetInput<1>().Set(0);
		node4.GetInput<0>().Set(0);
		node4.GetInput<1>().Set(0);

		gnode0.GetInput<0>().Set(0);
		gnode0.GetInput<1>().Set(0);
		gnode1.GetInput<0>().Set(0);
		gnode1.GetInput<1>().Set(0);

		// add a few links
		node0.GetOutput(0)->Link(node1.GetInput(0));
		node0.GetOutput(0)->Link(node2.GetInput(0));
		node0.GetOutput(0)->Link(node2.GetInput(1));
		node1.GetOutput(0)->Link(gnode0.GetInput(0));
		node2.GetOutput(0)->Link(node3.GetInput(0));
		gnode0.GetOutput(0)->Link(node4.GetInput(0));
		node2.GetOutput(0)->Link(gnode0.GetInput(1));
		node3.GetOutput(0)->Link(node4.GetInput(1));
		node4.GetOutput(0)->Link(gnode1.GetInput(0));

		// create pipeline
		pipeline.CreateFromNodesList({&node0, &node1, &node2, &node3, &node4, &gnode0, &gnode1}, Pipeline::NoDeleter());

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

		// draw task graph
		file.open("taskgraph.dot");
		lemon::ListDigraph::NodeMap<std::string> taskLabel(pipeline.GetTaskGraph());
		int taskIndex = 0;
		for (lemon::ListDigraph::NodeIt node(pipeline.GetTaskGraph()); node != lemon::INVALID; ++node) {
			bool isDummyTask = !(bool)pipeline.GetTaskFunctionMap()[node];
			stringstream ss;
			ss << (isDummyTask ? "dummy_" : "") << "task_" << taskIndex++;
			taskLabel[node] = ss.str();
		}
		Graphviz(pipeline.GetTaskGraph(), taskLabel, file);
		file.close();

		// execute pipeline
		//scheduler.SetPipeline(std::move(pipeline));


		//scheduler.Execute();
	}
	catch (std::exception& ex) {
		cout << "Failed to create pipeline: " << ex.what() << endl;
	}
	*/
	return 0;
}