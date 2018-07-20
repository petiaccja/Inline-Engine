#include "MaterialEditorNode.hpp"


namespace inl::gxeng {


MaterialEditorNode::MaterialEditorNode(std::unique_ptr<MaterialShader> realNode)
	: m_realNode(std::move(realNode)), m_metaData{ Vec2i{ 0,0 } }
{}

std::string MaterialEditorNode::GetName() const {
	return m_realNode->GetDisplayName();
}

void MaterialEditorNode::SetName(std::string name) {
	m_realNode->SetDisplayName(std::move(name));
}

std::string MaterialEditorNode::GetClassName() const {
	return m_realNode->GetClassName();
}

int MaterialEditorNode::GetNumInputs() const {
	return (int)m_realNode->GetNumInputs();
}

int MaterialEditorNode::GetNumOutputs() const {
	return (int)m_realNode->GetNumOutputs();
}

bool MaterialEditorNode::HasVariableInputs() const {
	return false;
}

bool MaterialEditorNode::HasVariableOutputs() const {
	return false;
}

void MaterialEditorNode::SetNumInputs() {
	throw InvalidCallException("Material nodes don't support variable number of inputs.");
}

void MaterialEditorNode::SetNumOutputs() {
	throw InvalidCallException("Material nodes don't support variable number of outputs.");
}

std::string MaterialEditorNode::GetInputName(int idx) const {
	return m_realNode->GetInputName(idx);
}

std::string MaterialEditorNode::GetOutputName(int idx) const {
	return m_realNode->GetOutputName(idx);
}

bool MaterialEditorNode::HasPortTypes() const {
	return false;
}

std::type_index MaterialEditorNode::GetInputType(int idx) const {
	throw InvalidCallException("This graph has no C++ port types.");
}

std::type_index MaterialEditorNode::GetOutputType(int idx) const {
	throw InvalidCallException("This graph has no C++ port types.");
}

std::string MaterialEditorNode::GetInputTypeName(int idx) const {
	return m_realNode->GetInput(idx)->GetType();
}

std::string MaterialEditorNode::GetOutputTypeName(int idx) const {
	return m_realNode->GetOutput(idx)->GetType();
}

MaterialShader* MaterialEditorNode::GetRealNode() const {
	return m_realNode.get();
}

void MaterialEditorNode::SetMetaData(NodeMetaDescription data) {
	m_metaData = data;
}

NodeMetaDescription MaterialEditorNode::GetMetaData() const {
	return m_metaData;
}


} // namespace inl::gxeng