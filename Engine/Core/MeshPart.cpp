#include "MeshPart.hpp"
#include "Core.hpp"

namespace inl::core {

MeshPart::MeshPart(gxeng::MeshEntity* e)
: Part(TYPE), entity(e)
{
}

gxeng::MeshEntity* MeshPart::GetEntity()
{
	return entity;
}

void MeshPart::SetTextureNormal(const std::string& contentPath)
{
	assert(0);
	//graphics::ITexture* texture = Core.GetGraphicsEngine()->CreateTexture();
	//texture->Load(GetAssetsDir() + contentPath);
	//
	//// Load contentPath
	//graphics::IMaterial* material = entity->GetMaterial();
	//
	//int nSubMaterials = material->GetNumSubMaterials();
	//
	//for (int i = 0; i < nSubMaterials; ++i)
	//{
	//	graphics::IMaterial::SubMaterial& subMaterial = material->GetSubMaterial(i);
	//
	//	subMaterial.t_normal = texture;
	//}
}

void MeshPart::SetTextureBaseColor(const std::string& contentPath)
{
	assert(0);

	//graphics::ITexture* texture = Core.GetGraphicsEngine()->CreateTexture();
	//texture->Load(GetAssetsDir() + contentPath);
	//
	//// Load contentPath
	//graphics::IMaterial* material = entity->GetMaterial();
	//
	//int nSubMaterials = material->GetNumSubMaterials();
	//
	//for (int i = 0; i < nSubMaterials; ++i)
	//{
	//	graphics::IMaterial::SubMaterial& subMaterial = material->GetSubMaterial(i);
	//
	//	subMaterial.t_diffuse = texture;
	//}
}

void MeshPart::SetTextureAO(const std::string& contentPath)
{
	assert(0);
	//graphics::ITexture* texture = Core.GetGraphicsEngine()->CreateTexture();
	//texture->Load(GetAssetsDir() + contentPath);
	//
	//// Load contentPath
	//graphics::IMaterial* material = entity->GetMaterial();
	//
	//int nSubMaterials = material->GetNumSubMaterials();
	//
	//for (int i = 0; i < nSubMaterials; ++i)
	//{
	//	graphics::IMaterial::SubMaterial& subMaterial = material->GetSubMaterial(i);
	//
	//	subMaterial.t_ao = texture;
	//}
}

} // namespace inl::core