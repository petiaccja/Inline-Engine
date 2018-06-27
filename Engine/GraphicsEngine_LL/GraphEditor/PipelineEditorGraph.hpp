#include "../GraphicsNodeFactory.hpp"

#include <BaseLibrary/GraphEditor/IGraph.hpp>
#include <BaseLibrary/Graph_All.hpp>

#include "PipelineEditorNode.hpp"


namespace inl::gxeng {


class PipelineEditorGraph : public IGraph {
public:
	PipelineEditorGraph(const GraphicsNodeFactory& factory);

	std::vector<std::string> GetNodeList() const override;

	INode* AddNode(std::string name) override;
	void RemoveNode(INode* node) override;

	inl::Link Link(INode* sourceNode, int sourcePort, INode* targetNode, int targetPort) override;
	void Unlink(INode* targetNode, int targetPort) override;

	std::vector<INode*> GetNodes() const override;
	std::vector<inl::Link> GetLinks() const override;

	void Validate() override;
	std::string SerializeJSON() override;
	void LoadJSON(const std::string& description) override;

private:
	const GraphicsNodeFactory& m_factory;
	std::vector<std::unique_ptr<PipelineEditorNode>> m_nodes;
};



} // namespace inl::gxeng