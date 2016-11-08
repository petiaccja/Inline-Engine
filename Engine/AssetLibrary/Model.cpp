#include "Model.hpp"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/mesh.h>

namespace inl {
namespace asset {

// count of nodes that contain at least one mesh
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

const aiNode* GetFirstFilledNode(const aiNode* node) {
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
		throw std::runtime_error("Could not load model \"" + filename + "\". Importer message:\n" + msg);
	}

	if (!m_scene->HasMeshes()) {
		throw std::runtime_error("Model was loaded successfully but it does not contain any meshes!");
	}

	if (FilledNodeCount(m_scene->mRootNode) > 1) {
		throw std::runtime_error("Model contains more than one mesh containing nodes. Only single noded models are supported at this time.");
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