#include "Test.hpp"
#include <thread>
#include <iostream>
#include "GraphicsEngine_LL/Vertex.hpp"
#include "GraphicsEngine_LL/VertexArrayView.hpp"

using namespace std::literals::chrono_literals;

using std::cout;
using std::endl;


//------------------------------------------------------------------------------
// Test class
//------------------------------------------------------------------------------


class TestVertex : public AutoRegisterTest<TestVertex> {
public:
	TestVertex() {}

	static std::string Name() {
		return "Vertex";
	}
	int Run() override;
private:
	static int a;
};


//------------------------------------------------------------------------------
// Test definition
//------------------------------------------------------------------------------


int TestVertex::Run() {
	using namespace inl::gxeng;

	// create a vertex
	using MyVertex1 = Vertex < Position<0>, Position<1>, Normal<0> >;

	MyVertex1 v;
	v.normal.x() = 6;

	static_cast<VertexPart<POSITION>*>(&v);

	VertexBase* pVertex = &v;
	auto* positionPart = dynamic_cast<VertexPart<POSITION>*>(pVertex);
	mathfu::Vector<float, 3>& pos0 = positionPart->GetPosition(0);

	// create a vertex array view
	VertexArrayView<VertexPart<POSITION>> view(&v, 1, sizeof(MyVertex1));
	const auto& cview = view;
	VertexPart<POSITION>& part = view[0];
	VertexPart<POSITION>& cpart = cview[0];

	// create a vertex array view to const
	VertexArrayView<const VertexPart<POSITION>> viewToConst(&v, 1, sizeof(MyVertex1));
	const VertexPart<POSITION>& part2 = viewToConst[0];

	// create const view from const
	VertexArrayView<const VertexPart<POSITION>> viewToConstFromConst((const MyVertex1*)&v, 1, sizeof(MyVertex1));

	// create view from const
	// VertexArrayView<VertexPart<POSITION>> viewFromConst((const MyVertex1*)&v, 1); // compile error


	return 0;
}