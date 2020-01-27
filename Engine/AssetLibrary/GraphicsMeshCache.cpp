#include "GraphicsMeshCache.hpp"

#include "Model.hpp"


namespace inl::asset {


GraphicsMeshCache::GraphicsMeshCache(gxeng::IGraphicsEngine& engine)
	: m_engine(engine) {}


std::shared_ptr<gxeng::IMesh> GraphicsMeshCache::Create(const std::filesystem::path& path) {
	Model model{ path };

	CoordSysLayout csys;
	csys.x = AxisDir::POS_X;
	csys.y = AxisDir::POS_Z;
	csys.z = AxisDir::NEG_Y;

	auto vertices = model.GetVertices<gxeng::Position<0>, gxeng::Normal<0>, gxeng::TexCoord<0>, gxeng::Tangent<0>>(0, csys);
	auto indices = model.GetIndices(0);

	std::shared_ptr<gxeng::IMesh> mesh(m_engine.CreateMesh());

	mesh->Set(vertices.data(), &vertices[0].GetReader(), vertices.size(), indices.data(), indices.size());

	return mesh;
}


} // namespace inl::asset