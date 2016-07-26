#include "Test.hpp"
#include <thread>
#include <iostream>
#include "GraphicsEngine_LL/Vertex.hpp"

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

	using MyVertex1 = Vertex < Position<0>, Position<1>, Normal<0> >;

	MyVertex1 v;
	v.normal.x() = 6;

	static_cast<VertexPart<POSITION>*>(&v);

	VertexBase* pVertex = &v;
	auto* positionPart = dynamic_cast<VertexPart<POSITION>*>(pVertex);
	mathfu::Vector<float, 3>& pos0 = positionPart->GetPosition(0);

	return 0;
}