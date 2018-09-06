#include "Test.hpp"
#include <thread>
#include <iostream>
#include <InlineMath.hpp>
#include <GraphicsEngine/Resources/Vertex.hpp>
#include "BaseLibrary/ArrayView.hpp"

using namespace std::literals::chrono_literals;
using namespace inl;

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
	using inl::ArrayView;

	// create a vertex
	using MyVertex1 = Vertex < Position<0>, Position<1>, Normal<0> >;

	MyVertex1 v;
	v.positions[0] = { 1,2,3 };
	v.normals[0] = { 0.7071f, 0.f, 0.7071f };
	
	auto* p1 = static_cast<inl::gxeng::impl::VertexPartRealization<Normal<0>>*>(&v);
	auto* p2 = static_cast<inl::gxeng::VertexPart<eVertexElementSemantic::NORMAL, 0>*>(&v);

	auto reader = decltype(v)::GetReader();
	const VertexPartReader<eVertexElementSemantic::POSITION>* posReader = reader.GetPartReader<eVertexElementSemantic::POSITION>();
	const VertexPartReader<eVertexElementSemantic::NORMAL>* normReader = reader.GetPartReader<eVertexElementSemantic::NORMAL>();

	auto positions = posReader->GetIndices();
	auto normals = normReader->GetIndices();

	Vec3 pos0 = posReader->GetPosition(v, 0);
	Vec3 pos1 = posReader->GetPosition(v, 1);
	Vec3 norm0 = normReader->GetNormal(v, 0);



	//static_cast<VertexPart<eVertexElementSemantic::POSITION>*>(&v);

	/*
	VertexBase* pVertex = &v;
	auto* positionPart = dynamic_cast<VertexPart<eVertexElementSemantic::POSITION>*>(pVertex);
	Vec3& pos0 = positionPart->GetPosition(0);

	// create a vertex array view
	ArrayView<VertexPart<eVertexElementSemantic::POSITION>> view(&v, 1, sizeof(MyVertex1));
	const auto& cview = view;
	VertexPart<eVertexElementSemantic::POSITION>& part = view[0];
	VertexPart<eVertexElementSemantic::POSITION>& cpart = cview[0];

	// create a vertex array view to const
	ArrayView<const VertexPart<eVertexElementSemantic::POSITION>> viewToConst(&v, 1, sizeof(MyVertex1));
	const VertexPart<eVertexElementSemantic::POSITION>& part2 = viewToConst[0];

	// create const view from const
	ArrayView<const VertexPart<eVertexElementSemantic::POSITION>> viewToConstFromConst((const MyVertex1*)&v, 1, sizeof(MyVertex1));

	// create view from const
	// VertexArrayView<VertexPart<POSITION>> viewFromConst((const MyVertex1*)&v, 1); // compile error

	// iterators
	decltype(viewToConstFromConst)::const_iterator it1 = viewToConstFromConst.begin();
	auto it3 = viewToConstFromConst.begin();
	decltype(view)::iterator it2 = view.begin();
	*/


	return 0;
}