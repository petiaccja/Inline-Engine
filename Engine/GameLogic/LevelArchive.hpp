#include <cereal/cereal.hpp>
#include "GraphicsEngine/IGraphicsEngine.hpp"


namespace inl::game {

struct ModuleEntityBindings {
	gxeng::IGraphicsEngine* graphicsEngine;
};





template <class Archive>
class LevelArchive : public Archive {
public:
	
private:
	
};



} // namespace inl::game