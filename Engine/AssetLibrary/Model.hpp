#pragma once

#include <GraphicsEngine/Resources/Vertex.hpp>

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>

#include <InlineMath.hpp>

#include <vector>
#include <memory>


namespace inl {
namespace asset {

enum class AxisDir : uint8_t { POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z };

Vec4 GetAxis(AxisDir dir);

struct CoordSysLayout {
	AxisDir x, y, z;
};

class Model {
public:
	Model();
	explicit Model(const std::string& filename);

	unsigned SubmeshCount() const;

	template <typename... AttribT>
	std::vector<gxeng::Vertex<AttribT...>> GetVertices(unsigned submeshID, CoordSysLayout cSysLayout = { AxisDir::POS_X, AxisDir::POS_Y, AxisDir::POS_Z }) const;

	std::vector<unsigned> GetIndices(unsigned submeshID) const;

protected:
	// It is cleary stated in the documentation that an imporer instance will keep ownership
	// of the imported scene. This is fine. But seems like an importer can only store one scene
	// at a time so an importer instance is inherently attached to a scene instance.
	std::shared_ptr<Assimp::Importer> m_importer;
	const aiScene* m_scene;

	Mat44 m_transform;
	Mat44 m_invTrTransform;

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

	auto xAxis = GetAxis(csys.x);
	auto yAxis = GetAxis(csys.y);
	auto zAxis = GetAxis(csys.z);
	const Mat44 posTransform =
		m_transform *
		//(Mat44(GetAxis(csys.x), GetAxis(csys.y), GetAxis(csys.z), Vec4(0, 0, 0, 1)).Transpose());
		Mat44(xAxis.x, yAxis.x, zAxis.x, 0,
			  xAxis.y, yAxis.y, zAxis.y, 0,
			  xAxis.z, yAxis.z, zAxis.z, 0,
			  0, 0, 0, 1);


	const Mat44 normalTransform = posTransform.Inverse().Transpose();

	for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
		VertexT newVertex;
		VertexAttributeSetter<VertexT, AttribT...>()(newVertex, mesh, i, posTransform, normalTransform);
		result.push_back(newVertex);
	}

	return result;
}


template <typename VertexT>
struct Model::VertexAttributeSetter<VertexT> {
	inline void operator()(VertexT&, const aiMesh*, uint32_t, const Mat44&, const Mat44&) {}
};


template <typename VertexT, int semanticIndex, typename... TailAttribT>
struct Model::VertexAttributeSetter<VertexT, gxeng::Position<semanticIndex>, TailAttribT...> {
	static_assert(semanticIndex == 0, "There is only one position attribute inside a model.");
	inline void operator()(
		VertexT& target,
		const aiMesh* mesh,
		uint32_t vertexIndex,
		const Mat44& posTr,
		const Mat44& normTr
		) {
		using DataType = gxeng::VertexPartReader<gxeng::eVertexElementSemantic::POSITION>::DataType;
		assert(mesh->HasPositions());
		assert(vertexIndex < mesh->mNumVertices);
		const aiVector3D& pos = mesh->mVertices[vertexIndex];
		target.position = (posTr * Vec4(pos.x, pos.y, pos.z, 1)).xyz;

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
		const Mat44& posTr,
		const Mat44& normTr
		) {
		using DataType = gxeng::VertexPartReader<gxeng::eVertexElementSemantic::NORMAL>::DataType;
		if (mesh->HasNormals() == false) {
			throw InvalidCallException("Vertex array requested with normals but loaded mesh does not have such an attribute.");
		}
		assert(vertexIndex < mesh->mNumVertices);
		const aiVector3D& normal = mesh->mNormals[vertexIndex];
		target.normal = normTr * (Vec3)DataType(normal.x, normal.y, normal.z);
		//target.normal = DataType(normal.x, normal.y, normal.z);

		VertexAttributeSetter<VertexT, TailAttribT...>()(target, mesh, vertexIndex, posTr, normTr);
	}
};


template <typename VertexT, int semanticIndex, typename... TailAttribT>
struct Model::VertexAttributeSetter<VertexT, gxeng::Tangent<semanticIndex>, TailAttribT...> {
	static_assert(semanticIndex == 0, "There is only one \"tangent vector\" attribute inside a model.");
	inline void operator()(
		VertexT& target,
		const aiMesh* mesh,
		uint32_t vertexIndex,
		const Mat44& posTr,
		const Mat44& normTr
		) {
		using DataType = typename gxeng::VertexPart<gxeng::eVertexElementSemantic::NORMAL>::DataType;
		if (mesh->HasTangentsAndBitangents() == false) {
			throw InvalidCallException("Vertex array requested with tangents but loaded mesh does not have such an attribute.");
		}
		assert(vertexIndex < mesh->mNumVertices);
		const aiVector3D& tangent = mesh->mTangents[vertexIndex];
		target.tangent = normTr * DataType(tangent.x, tangent.y, tangent.z);

		VertexAttributeSetter<VertexT, TailAttribT...>()(target, mesh, vertexIndex, posTr, normTr);
	}
};


template <typename VertexT, int semanticIndex, typename... TailAttribT>
struct Model::VertexAttributeSetter<VertexT, gxeng::Bitangent<semanticIndex>, TailAttribT...> {
	static_assert(semanticIndex == 0, "There is only one \"bitangent vector\" attribute inside a model.");
	inline void operator()(
		VertexT& target,
		const aiMesh* mesh,
		uint32_t vertexIndex,
		const Mat44& posTr,
		const Mat44& normTr
		) {
		using DataType = typename gxeng::VertexPart<gxeng::eVertexElementSemantic::NORMAL>::DataType;
		if (mesh->HasTangentsAndBitangents() == false) {
			throw InvalidCallException("Vertex array requested with bitangents but loaded mesh does not have such an attribute.");
		}
		assert(vertexIndex < mesh->mNumVertices);
		const aiVector3D& bitangent = mesh->mBitangents[vertexIndex];
		target.bitangent = normTr * DataType(bitangent.x, bitangent.y, bitangent.z);

		VertexAttributeSetter<VertexT, TailAttribT...>()(target, mesh, vertexIndex, posTr, normTr);
	}
};


template <typename VertexT, int semanticIndex, typename... TailAttribT>
struct Model::VertexAttributeSetter<VertexT, gxeng::TexCoord<semanticIndex>, TailAttribT...> {
	inline void operator()(
		VertexT& target,
		const aiMesh* mesh,
		uint32_t vertexIndex,
		const Mat44& posTr,
		const Mat44& normTr
		) {
		using DataType = gxeng::VertexPartReader<gxeng::eVertexElementSemantic::TEX_COORD>::DataType;
		if (mesh->HasTextureCoords(semanticIndex) == false) {
			throw InvalidCallException(
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
		const Mat44& posTr,
		const Mat44& normTr
		) {
		using DataType = gxeng::VertexPartReader<gxeng::eVertexElementSemantic::COLOR>::DataType;
		if (mesh->HasVertexColors(semanticIndex) == false) {
			throw InvalidCallException(
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
