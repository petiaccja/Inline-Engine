#include "CreateTexture.hpp"
#include "GetBackBuffer.hpp"
#include "GetCamera2DByName.hpp"
#include "GetCameraByName.hpp"
#include "GetEnvVariable.hpp"
#include "GetSceneByName.hpp"
#include "GetTime.hpp"

#include "../../GraphicsNodeFactory.hpp"

extern "C"
bool g_autoRegisterSysNodes = [] {
	using namespace inl::gxeng;

	GraphicsNodeFactory_Singleton::GetInstance().RegisterNodeClass<nodes::CreateTexture>("");
	GraphicsNodeFactory_Singleton::GetInstance().RegisterNodeClass<nodes::GetBackBuffer>("");
	GraphicsNodeFactory_Singleton::GetInstance().RegisterNodeClass<nodes::GetCamera2DByName>("");
	GraphicsNodeFactory_Singleton::GetInstance().RegisterNodeClass<nodes::GetCameraByName>("");
	GraphicsNodeFactory_Singleton::GetInstance().RegisterNodeClass<nodes::GetEnvVariable>("");
	GraphicsNodeFactory_Singleton::GetInstance().RegisterNodeClass<nodes::GetSceneByName>("");
	GraphicsNodeFactory_Singleton::GetInstance().RegisterNodeClass<nodes::GetTime>("");

	return true;
}();


