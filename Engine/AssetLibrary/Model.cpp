#include "Model.hpp"

#include <BaseLibrary/Exception/Exception.hpp>

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>

namespace inl {
namespace asset {

// Number of nodes that contain at least one mesh
static unsigned FilledNodeCount(const aiNode* node) {
	unsigned result = 0;
	if (node->mNumMeshes > 0) {
		result++;
	}
	for (unsigned i = 0; i < node->mNumChildren; i++) {
		result += FilledNodeCount(node->mChildren[i]);
	}
	return result;
}

static const aiNode* GetFirstFilledNode(const aiNode* node) {
	if (node->mNumMeshes > 0) {
		return node;
	}
	for (unsigned i = 0; i < node->mNumChildren; i++) {
		const aiNode* match = GetFirstFilledNode(node->mChildren[i]);
		if (match != nullptr) {
			return match;
		}
	}
	return nullptr;
}

static Mat44 GetAbsoluteTransform(const aiNode* node) {
	const auto& m = node->mTransformation;
	// this constructor expects elements to be in column-major order
	// so no pretty formatting is possible that visualizes how elemnts are in the matrix -_-
	Mat44 currTransform{
		m.a1, m.a2, m.a3, m.a4, m.b1, m.b2, m.b3, m.b4, m.c1, m.c2, m.c3, m.c4, m.d1, m.d2, m.d3, m.d4
	};
	if (node->mParent == nullptr) {
		return currTransform;
	}
	return currTransform * GetAbsoluteTransform(node->mParent);
}


Vec4 GetAxis(AxisDir dir) {
	switch (dir) {
	case AxisDir::POS_X:
		return Vec4(+1, 0, 0, 0);
	case AxisDir::NEG_X:
		return Vec4(-1, 0, 0, 0);
	case AxisDir::POS_Y:
		return Vec4(0, +1, 0, 0);
	case AxisDir::NEG_Y:
		return Vec4(0, -1, 0, 0);
	case AxisDir::POS_Z:
		return Vec4(0, 0, +1, 0);
	case AxisDir::NEG_Z:
		return Vec4(0, 0, -1, 0);
	default:
		assert(false);
	}

	return Vec4(0, 0, 0, 0);
}


Model::Model() :
	m_scene(nullptr)
{}


Model::Model(const std::string & filename) {
	m_importer.reset(new Assimp::Importer);
	// "aiProcess_OptimizeGraph" will collapse nodes if possible.
	// This flag is used to have ideally all submeshes in a single node.
	m_scene = m_importer->ReadFile(filename, aiProcessPreset_TargetRealtime_Quality | aiProcess_OptimizeGraph);

	if (m_scene == nullptr) {
		const std::string msg(m_importer->GetErrorString());
		throw RuntimeException("Could not load model \"" + filename + "\".",  msg);
	}

	if (!m_scene->HasMeshes()) {
		throw InvalidArgumentException("Model was loaded successfully but it does not contain any meshes!");
	}

	if (FilledNodeCount(m_scene->mRootNode) > 1) {
		throw InvalidArgumentException("Model contains more than one mesh containing nodes. Only single noded models are supported at this time.");
	}
	// Find the single node that contains meshes.
	const aiNode* node = GetFirstFilledNode(m_scene->mRootNode);

	m_transform = GetAbsoluteTransform(node);
	m_invTrTransform = m_transform.Inverse().Transpose();
}


unsigned Model::SubmeshCount() const {
	assert(m_scene != nullptr);
	return m_scene->mNumMeshes;
}


std::vector<unsigned> Model::GetIndices(unsigned submeshID) const {
	unsigned meshCount = m_scene->mNumMeshes;
	assert(submeshID < meshCount);

	const aiMesh* mesh = m_scene->mMeshes[submeshID];

	unsigned faceCount = mesh->mNumFaces;
	std::vector<unsigned> indices;
	indices.reserve(faceCount);

	for (unsigned faceID = 0; faceID < faceCount; faceID++) {
		const aiFace& currFace = mesh->mFaces[faceID];
		for (unsigned i = 0; i < currFace.mNumIndices; i++) {
			indices.push_back(currFace.mIndices[i]);
		}
	}

	return indices;
}



} // namespace asset
} // namespace inl
