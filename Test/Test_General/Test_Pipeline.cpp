#include "Test.hpp"

#include <GraphicsEngine_LL/GraphicsNodeFactory.hpp>
#include <GraphicsEngine_LL/Pipeline.hpp>
#include <GraphicsEngine_LL/GraphicsNode.hpp>
#include <BaseLibrary/Graph_All.hpp>

#include <iostream>

using namespace std;
using namespace inl::gxeng;
using namespace exc;


class TestNode : public InputPortConfig<int>, public OutputPortConfig<int> {
public:
	static std::string Info_GetName() { return "TestNode"; }
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

	mutable int id;
};

class TestGraphicsNode : public GraphicsNode, public InputPortConfig<int>, public OutputPortConfig<int> {
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
	Task GetTask() override {
		return [this](ExecutionContext) {this->Update(); return ExecutionResult{};};
	}

	mutable int id;
};




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


int TestPipeline::Run() {
	cout << "Creating factory..." << endl;

	GraphicsNodeFactory factory;
	factory.RegisterNodeClass<TestNode>("");
	factory.RegisterNodeClass<TestGraphicsNode>("");

	cout << "Creating pipeline..." << endl;

	Pipeline pipeline;
	pipeline.SetFactory(&factory);

	try {
		for (int i = 1; i <= 5; i++) {
			auto it = pipeline.AddNode("TestNode");
			dynamic_cast<const TestNode&>(*it).id = i;
		}

		std::vector<Pipeline::NodeIterator> nodes;
		for (auto it = pipeline.Begin(); it != pipeline.End(); ++it) {
			nodes.push_back(it);
		}
		std::sort(nodes.begin(), nodes.end(), [](auto lhs, auto rhs) {
			return dynamic_cast<const TestNode&>(*lhs).id < dynamic_cast<const TestNode&>(*rhs).id;
		});
		
		Pipeline::NodeIterator prevIt = pipeline.End();
		for (auto& node : nodes) {
			if (prevIt != pipeline.End()) {
				pipeline.AddLink(prevIt, 0, node, 0);
			}
			prevIt = node;

			cout << dynamic_cast<const TestNode&>(*node).id << endl;
		}

	}
	catch (std::exception& ex) {
		cout << "Failed to create pipeline: " << ex.what() << endl;
	}

	return 0;
}