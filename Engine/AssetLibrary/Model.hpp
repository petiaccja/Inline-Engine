#pragma once

#include <GraphicsEngine_LL/Vertex.hpp>

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>

#include <mathfu/mathfu_exc.hpp>

#include <vector>
#include <memory>


namespace inl {
namespace asset {

enum class AxisDir : uint8_t { POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z };

mathfu::Vector4f GetAxis(AxisDir dir);

struct CoordSysLayout {
	AxisDir x, y, z;
};

class Model {
public:
	Model();
	explicit Model(const std::string& filename);

	unsigned SubmeshCount() const;

	template <typename... AttribT>
	std::vector<gxeng::Vertex<AttribT...>> GetVertices(unsigned submeshID, CoordSysLayout cSysLayout = {AxisDir::POS_X, AxisDir::POS_Y, AxisDir::POS_Z}) const;

	std::vector<unsigned> GetIndices(unsigned submeshID) const;

protected:
	// It is cleary stated in the documentation that an imporer instance will keep ownership
	// of the imported scene. This is fine. But seems like an importer can only store one scene
	// at a time so an importer instance is inherently attached to a scene instance.
	std::shared_ptr<Assimp::Importer> m_importer; 
	const aiScene* m_scene;
	
	mathfu::Matrix<float, 4, 4> m_transform;
	mathfu::Matrix<float, 4, 4> m_invTrTransform;

private:
	
	template <typename VertexT, typename... AttribsT>
	struct VertexAttributeSetter;
};



template <typename... AttribT>
inline std::vector<gxeng::Vertex<AttribT...>> Model::GetVertices(unsigned submeshID, CoordSysLayout csys) const {
	using VertexT = gxeng::Vertex<AttribT...>;
	std::vector<VertexT> result;
	
	assert(submeshID < m_scene->mNumMeshes);
	const aiMesh* mesh = m_scene->mMeshes[submeshID];
	result.reserve(mesh->mNumVertices);

	const mathfu::Matrix4x4f posTransform =
		m_transform *
		(mathfu::Matrix4x4f(GetAxis(csys.x), GetAxis(csys.y), GetAxis(csys.z), mathfu::Vector4f(0, 0, 0, 1)).Transpose());

	const mathfu::Matrix4x4f normalTransform = posTransform.Inverse().Transpose();

	for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
		VertexT newVertex;
		VertexAttributeSetter<VertexT, AttribT...>()(newVertex, mesh, i, posTransform, normalTransform);
		result.push_back(newVertex);
	}

	return result;
}


template <typename VertexT>
struct Model::VertexAttributeSetter<VertexT> {
	inline void operator()(VertexT&, const aiMesh*, uint32_t, const mathfu::Matrix4x4f&, const mathfu::Matrix4x4f&) {}
};


template <typename VertexT, int semanticIndex, typename... TailAttribT>
struct Model::VertexAttributeSetter<VertexT, gxeng::Position<semanticIndex>, TailAttribT...> {
	static_assert(semanticIndex == 0, "There is only one position attribute inside a model.");
	inline void operator()(
		VertexT& target,
		const aiMesh* mesh,
		uint32_t vertexIndex,
		const mathfu::Matrix4x4f& posTr,
		const mathfu::Matrix4x4f& normTr
	) {
		using DataType = gxeng::VertexPart<gxeng::eVertexElementSemantic::POSITION>::DataType;
		assert(mesh->HasPositions());
		assert(vertexIndex < mesh->mNumVertices);
		const aiVector3D& pos = mesh->mVertices[vertexIndex];
		//target.position = (model->m_transform * mathfu::Vector<float, 4>(pos.x, pos.z, -pos.y, 1)).xyz();
		target.position = (posTr * mathfu::Vector<float, 4>(pos.x, pos.y, pos.z, 1)).xyz();
		//target.position = DataType(pos.x, pos.z, -pos.y);

		VertexAttributeSetter<VertexT, TailAttribT...>()(target, mesh, vertexIndex, posTr, normTr);
	}
};


template <typename VertexT, int semanticIndex, typename... TailAttribT>
struct Model::VertexAttributeSetter<VertexT, gxeng::Normal<semanticIndex>, TailAttribT...> {
	static_assert(semanticIndex == 0, "There is only one \"normal vector\" attribute inside a model.");
	inline void operator()(
		VertexT& target,
		const aiMesh* mesh,
		uint32_t vertexIndex,
		const mathfu::Matrix4x4f& posTr,
		const mathfu::Matrix4x4f& normTr
	) {
		using DataType = gxeng::VertexPart<gxeng::eVertexElementSemantic::NORMAL>::DataType;
		if (mesh->HasNormals() == false) {
			throw std::runtime_error("Vertex array requested with normals but loaded mesh does not have such an attribute.");
		}
		assert(vertexIndex < mesh->mNumVertices);
		const aiVector3D& normal = mesh->mNormals[vertexIndex];
		target.normal = normTr * DataType(normal.x, normal.y, normal.z);
		//target.normal = DataType(normal.x, normal.y, normal.z);

		VertexAttributeSetter<VertexT, TailAttribT...>()(target, mesh, vertexIndex, posTr, normTr);
	}
};


template <typename VertexT, int semanticIndex, typename... TailAttribT>
struct Model::VertexAttributeSetter<VertexT, gxeng::TexCoord<semanticIndex>, TailAttribT...> {
	inline void operator()(
		VertexT& target,
		const aiMesh* mesh,
		uint32_t vertexIndex,
		const mathfu::Matrix4x4f& posTr,
		const mathfu::Matrix4x4f& normTr
	) {
		using DataType = gxeng::VertexPart<gxeng::eVertexElementSemantic::TEX_COORD>::DataType;
		if (mesh->HasTextureCoords(semanticIndex) == false) {
			throw std::runtime_error(
				"Vertex array requested with texture coords of semantic index "
				+ std::to_string(semanticIndex)
				+ " but loaded mesh does not have such an attribute with that semantic index.");
		}
		assert(vertexIndex < mesh->mNumVertices);
		const aiVector3D& texCoords = mesh->mTextureCoords[semanticIndex][vertexIndex];
		target.texCoord = DataType(texCoords.x, texCoords.y);

		VertexAttributeSetter<VertexT, TailAttribT...>()(target, mesh, vertexIndex, posTr, normTr);
	}
};


template <typename VertexT, int semanticIndex, typename... TailAttribT>
struct Model::VertexAttributeSetter<VertexT, gxeng::Color<semanticIndex>, TailAttribT...> {
	inline void operator()(
		VertexT& target,
		const aiMesh* mesh,
		uint32_t vertexIndex,
		const mathfu::Matrix4x4f& posTr,
		const mathfu::Matrix4x4f& normTr
	) {
		using DataType = gxeng::VertexPart<gxeng::eVertexElementSemantic::COLOR>::DataType;
		if (mesh->HasVertexColors(semanticIndex) == false) {
			throw std::runtime_error(
				"Vertex array requested with vertex colors of semantic index "
				+ std::to_string(semanticIndex)
				+ " but loaded mesh does not have such an attribute with that semantic index.");
		}
		assert(vertexIndex < mesh->mNumVertices);
		const aiColor4D& color = mesh->mColors[semanticIndex][vertexIndex];
		target.color = DataType(color.r, color.g, color.b);

		VertexAttributeSetter<VertexT, TailAttribT...>()(target, mesh, vertexIndex, posTr, normTr);
	}
};


} // namespace asset
} // namespace inl
