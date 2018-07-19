#include <BaseLibrary/GraphEditor/IEditorGraph.hpp>
#include <BaseLibrary/Graph_All.hpp>

#include "MaterialEditorNode.hpp"


namespace inl::gxeng {

class ShaderManager;


class MaterialEditorGraph : public IEditorGraph {
public:
	MaterialEditorGraph(const ShaderManager& shaderManager);

	std::vector<std::string> GetNodeList() const override;
	void SetNodeList(const std::vector<std::string>&) override;

	IGraphEditorNode* AddNode(std::string name) override;
	void RemoveNode(IGraphEditorNode* node) override;

	inl::Link Link(IGraphEditorNode* sourceNode, int sourcePort, IGraphEditorNode* targetNode, int targetPort) override;
	void Unlink(IGraphEditorNode* targetNode, int targetPort) override;

	std::vector<IGraphEditorNode*> GetNodes() const override;
	std::vector<inl::Link> GetLinks() const override;

	void Validate() override;
	std::string SerializeJSON() override;
	void LoadJSON(const std::string& description) override;

	const std::string& GetContentType() const override;
	void Clear() override;

private:
	const ShaderManager& m_shaderManager;
	std::vector<std::string> m_nodeList;
	std::vector<std::unique_ptr<MaterialEditorNode>> m_nodes;
};


} // namespace inl::gxeng