#include <GameLogic/System.hpp>
#include <GraphicsEngine/IGraphicsEngine.hpp>


namespace inl::gamelib {


class RenderingSystem : public game::SpecificSystem<RenderingSystem> {
public:
	RenderingSystem(gxeng::IGraphicsEngine* engine);
	void Update(float elapsed) override;
private:
	gxeng::IGraphicsEngine* m_engine = nullptr;
};


} // namespace inl::gamelib