#include "Test.hpp"
#include <iostream>
#include "GraphicsEngine_LL/Binder.hpp"

using std::cout;
using std::endl;

using namespace inl::gxeng;


//------------------------------------------------------------------------------
// Test class
//------------------------------------------------------------------------------


class TestBinder : public AutoRegisterTest<TestBinder> {
public:
	TestBinder() {}

	static std::string Name() {
		return "Binder";
	}
	virtual int Run() override;
private:
	static int a;
};


//------------------------------------------------------------------------------
// Test definition
//------------------------------------------------------------------------------

//#define USE_COUNTER_CHECK


int TestBinder::Run() {
	BindParameterDesc p;

	std::vector<BindParameterDesc> parameters;

	p.parameter.type = eBindParameterType::CONSTANT;
	p.parameter.reg = 0;
	p.parameter.space = 0;
	p.constantSize = 80;
	parameters.push_back(p);

	p.parameter.type = eBindParameterType::CONSTANT;
	p.parameter.reg = 10;
	p.parameter.space = 0;
	p.constantSize = 4;
	parameters.push_back(p);

	p.parameter.type = eBindParameterType::CONSTANT;
	p.parameter.reg = 20;
	p.parameter.space = 0;
	p.constantSize = 0;
	parameters.push_back(p);

	p.parameter.type = eBindParameterType::CONSTANT;
	p.parameter.reg = 30;
	p.parameter.space = 0;
	p.constantSize = 7;
	parameters.push_back(p);

	p.parameter.type = eBindParameterType::TEXTURE;
	p.parameter.reg = 0;
	p.parameter.space = 0;
	p.constantSize = 0;
	parameters.push_back(p);

	p.parameter.type = eBindParameterType::UNORDERED;
	p.parameter.reg = 0;
	p.parameter.space = 0;
	p.constantSize = 0;
	parameters.push_back(p);

	p.parameter.type = eBindParameterType::TEXTURE;
	p.parameter.reg = 1;
	p.parameter.space = 0;
	p.constantSize = 0;
	parameters.push_back(p);

	p.parameter.type = eBindParameterType::CONSTANT;
	p.parameter.reg = 40;
	p.parameter.space = 0;
	p.constantSize = 50;
	parameters.push_back(p);

	p.parameter.type = eBindParameterType::CONSTANT;
	p.parameter.reg = 30;
	p.parameter.space = 0;
	p.constantSize = 51;
	parameters.push_back(p);

	p.parameter.type = eBindParameterType::CONSTANT;
	p.parameter.reg = 40;
	p.parameter.space = 0;
	p.constantSize = 52;
	parameters.push_back(p);

	p.parameter.type = eBindParameterType::CONSTANT;
	p.parameter.reg = 30;
	p.parameter.space = 0;
	p.constantSize = 53;
	parameters.push_back(p);

	p.parameter.type = eBindParameterType::CONSTANT;
	p.parameter.reg = 30;
	p.parameter.space = 0;
	p.constantSize = 53;
	parameters.push_back(p);

	throw std::invalid_argument("Binder requires a graphics api, so it's no longer testable this way. Create a mock graphics api.");
	//Binder binder(parameters);

	//cout << binder;


	return 0;
}