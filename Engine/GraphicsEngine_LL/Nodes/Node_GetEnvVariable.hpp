#pragma once

#include "../GraphicsNode.hpp"

#include "../Scene.hpp"
#include "../DirectionalLight.hpp"

#include <BaseLibrary/Any.hpp>


namespace inl::gxeng::nodes {


/// <summary>
/// Get reference to a Scene identified by its name.
/// Inputs: name of the env var.
/// Outputs: value of the env var
/// </summary>
/// <remarks>
/// Throws an exception if the env var cannot be found, never returns nulls.
/// </remarks>
class GetEnvVariable :
	virtual public GraphicsNode,
	virtual public GraphicsTask,
	virtual public exc::InputPortConfig<std::string>,
	virtual public exc::OutputPortConfig<exc::Any>
{
public:
	GetEnvVariable() {}

	void Update() override {}

	void Notify(exc::InputPortBase* sender) override {}

	void Initialize(EngineContext& context) override {
		GraphicsNode::SetTaskSingle(this);
	}
	void Reset() override {}

	void Setup(SetupContext& context) {
		std::string varName = this->GetInput<0>().Get();
		
		if (m_envVariables) {
			auto it = m_envVariables->find(varName);
			if (it != m_envVariables->end()) {
				this->GetOutput<0>().Set(it->second);
				return;
			}
		}
		throw std::invalid_argument("Environment variable name \"" + varName + "\" not found.");		
	}

	void Execute(RenderContext& context) {}


	void SetEnvVariableList(const std::unordered_map<std::string, exc::Any>* envVars) {
		m_envVariables = envVars;
	}
	const std::unordered_map<std::string, exc::Any>* SetEnvVariableList() const {
		return m_envVariables;
	}
private:
	const std::unordered_map<std::string, exc::Any>* m_envVariables;
};


} // namespace inl::gxeng::nodes
