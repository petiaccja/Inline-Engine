#include "Test.hpp"

#include "GraphicsApi_D3D12/GxapiManager.hpp"
#include "GraphicsApi_LL/IFence.hpp"
#include "GraphicsApi_LL/IGraphicsApi.hpp"

#include <iostream>
#include <thread>

using namespace std::literals::chrono_literals;

using std::cout;
using std::endl;


//------------------------------------------------------------------------------
// Test class
//------------------------------------------------------------------------------


class TestGapiSync : public AutoRegisterTest<TestGapiSync> {
public:
	TestGapiSync() {}

	static std::string Name() {
		return "Gapi Sync";
	}
	virtual int Run() override;

private:
	static int a;
};


//------------------------------------------------------------------------------
// Test definition
//------------------------------------------------------------------------------


int TestGapiSync::Run() {
	cout << "Creating stuff..." << endl;
	std::unique_ptr<inl::gxapi::IGxapiManager> gxapiManager(new inl::gxapi_dx12::GxapiManager());
	std::unique_ptr<inl::gxapi::IGraphicsApi> graphicsApi(gxapiManager->CreateGraphicsApi(0));
	std::unique_ptr<inl::gxapi::IFence> fence(graphicsApi->CreateFence(0));

	std::thread th([&] {
		std::this_thread::sleep_for(500ms);
		fence->Wait(2);
		cout << "[worker] Fence signaled: " << fence->Fetch() << endl;
	});

	cout << "Fence initial value: " << fence->Fetch() << endl;
	cout << "Signaling fence with 2." << endl;
	fence->Signal(2);
	std::this_thread::sleep_for(200ms);
	cout << "Signaling fence with 3." << endl;
	fence->Signal(3);
	cout << "Fence value: " << fence->Fetch() << endl;


	th.join();

	return 0;
}