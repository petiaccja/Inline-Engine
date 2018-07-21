#include "Node_RenderForwardSimple.hpp"


namespace inl::gxeng::nodes {


void RenderForwardSimple::Reset() {
	GetInput(0)->Clear();
	GetInput(1)->Clear();
}


void RenderForwardSimple::Setup(SetupContext& context) {
	
}


void RenderForwardSimple::Execute(RenderContext& context) {
	
}



const std::string& RenderForwardSimple::GetInputName(size_t index) const {
	static const std::vector<std::string> names = {
		"Target",
		"Depth",
		"Camera",
		"Entities",
		"Lights",
	};
	return names[index];
}


const std::string& RenderForwardSimple::GetOutputName(size_t index) const {
	static const std::vector<std::string> names = {
		"Target",
		"Depth",
	};
	return names[index];
}





} // namespace inl::gxeng
