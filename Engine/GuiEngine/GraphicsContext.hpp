#include <GraphicsEngine/Scene/IScene.hpp>
#include <GraphicsEngine/IGraphicsEngine.hpp>


namespace inl::gui {


struct GraphicsContext {
public:
	gxeng::IGraphicsEngine* engine = nullptr;
	gxeng::IScene* scene = nullptr;
};


} // namespace inl::gui