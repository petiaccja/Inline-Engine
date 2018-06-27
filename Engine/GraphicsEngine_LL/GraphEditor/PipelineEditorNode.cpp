#include "PipelineEditorNode.hpp"


namespace inl::gxeng {


PipelineEditorNode::PipelineEditorNode(std::unique_ptr<NodeBase> realNode)
	: m_realNode(std::move(realNode)), m_metaData{Vec2i{0,0}}
{}

std::string PipelineEditorNode::GetName() const {
	return m_realNode->GetDisplayName();
}

void PipelineEditorNode::SetName(std::string name) {
	m_realNode->SetDisplayName(std::move(name));
}

std::string PipelineEditorNode::GetClassName() const {
	return m_realNode->GetClassName();
}

int PipelineEditorNode::GetNumInputs() const {
	return (int)m_realNode->GetNumInputs();
}

int PipelineEditorNode::GetNumOutputs() const {
	return (int)m_realNode->GetNumOutputs();
}

bool PipelineEditorNode::HasVariableInputs() const {
	return false;
}

bool PipelineEditorNode::HasVariableOutputs() const {
	return false;
}

void PipelineEditorNode::SetNumInputs() {
	throw InvalidCallException("You can't call this on graphics nodes. At least for now.");
}

void PipelineEditorNode::SetNumOutputs() {
	throw InvalidCallException("You can't call this on graphics nodes. At least for now.");
}

std::string PipelineEditorNode::GetInputName(int idx) const {
	return m_realNode->GetInputName(idx);
}

std::string PipelineEditorNode::GetOutputName(int idx) const {
	return m_realNode->GetOutputName(idx);
}

bool PipelineEditorNode::HasPortTypes() const {
	return true;
}

std::type_index PipelineEditorNode::GetInputType(int idx) const {
	return m_realNode->GetInput(idx)->GetType();
}

std::type_index PipelineEditorNode::GetOutputType(int idx) const {
	return m_realNode->GetOutput(idx)->GetType();
}

std::string PipelineEditorNode::GetInputTypeName(int idx) const {
	return GetInputType(idx).name();
}

std::string PipelineEditorNode::GetOutputTypeName(int idx) const {
	return GetOutputType(idx).name();
}

NodeBase* PipelineEditorNode::GetRealNode() const {
	return m_realNode.get();
}

void PipelineEditorNode::SetMetaData(NodeMetaDescription data) {
	m_metaData = data;
}

NodeMetaDescription PipelineEditorNode::GetMetaData() const {
	return m_metaData;
}


} // namespace inl::gxeng