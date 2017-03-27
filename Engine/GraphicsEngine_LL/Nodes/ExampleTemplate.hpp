#pragma once


#include "../GraphicsNode.hpp"
#include "../PipelineTypes.hpp"


namespace inl::gxeng::nodes {


class ExampleNode 
	: virtual public GraphicsNode,
	virtual public exc::InputPortConfig<pipeline::Texture2D>,
	virtual public exc::OutputPortConfig<float, int>
{
public:
	Task GetTask() override;


	virtual void Update() override {}


	virtual void Notify(exc::InputPortBase* sender) override {}


	void InitGraphics(const GraphicsContext&) override {}
private:
	void CreateResources(const GraphicsContext& context);

};






} // namespace inl::gxeng::nodes