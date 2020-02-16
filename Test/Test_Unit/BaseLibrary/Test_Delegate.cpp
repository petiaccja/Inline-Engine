#include <BaseLibrary/Delegate.hpp>

#include <Catch2/catch.hpp>
#include <array>

using namespace inl;


class Base1 {
public:
	virtual ~Base1() {}
};

class Base2 {
public:
	virtual ~Base2() {}
};

class Test : public virtual Base1, public Base2 {
public:
	virtual ~Test() {}
	void Foo(float a, float b) {}
};


TEST_CASE("Delegate compare", "[Delegate]") {
	Test test;
	using MyDelegate = Delegate<void(float, float)>;
	std::vector<unsigned char> area1(sizeof(MyDelegate));
	std::vector<unsigned char> area2(sizeof(MyDelegate));
	MyDelegate& del1 = *reinterpret_cast<MyDelegate*>(area1.data());
	MyDelegate& del2 = *reinterpret_cast<MyDelegate*>(area2.data());

	for (int i = 0; i < area1.size(); ++i) {
		area1[i] = i + 65;
		area2[i] = i + 65;
	}

	new (&del1) MyDelegate(&Test::Foo, &test);
	new (&del2) MyDelegate(&Test::Foo, &test);
	
	REQUIRE(del1 == del2);
}