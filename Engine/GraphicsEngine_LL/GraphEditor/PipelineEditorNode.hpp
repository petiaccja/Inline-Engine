#include <BaseLibrary/GraphEditor/INode.hpp>
#include <BaseLibrary/Graph_All.hpp>


#undef GetClassName // retarded Windows


namespace inl::gxeng {


class PipelineEditorNode : public INode {
public:
	PipelineEditorNode() = default;
	PipelineEditorNode(std::unique_ptr<NodeBase> realNode);
	
	std::string GetName() const override;
	void SetName(std::string name) override;
	std::string GetClassName() const override;

	int GetNumInputs() const override;
	int GetNumOutputs() const override;

	bool HasVariableInputs() const override;
	bool HasVariableOutputs() const override;
	void SetNumInputs() override;
	void SetNumOutputs() override;

	std::string GetInputName(int idx) const override;
	std::string GetOutputName(int idx) const override;

	bool HasPortTypes() const override;
	std::type_index GetInputType(int idx) const override;
	std::type_index GetOutputType(int idx) const override;

	std::string GetInputTypeName(int idx) const override;
	std::string GetOutputTypeName(int idx) const override;

	NodeBase* GetRealNode() const;
private:
	std::unique_ptr<NodeBase> m_realNode;
};



} // namespace inl