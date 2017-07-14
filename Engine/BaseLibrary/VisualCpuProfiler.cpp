#include "VisualCpuProfiler.h"
#include <sstream>
#include <iomanip>
//#include "GL\glew.h"
//#include "PlatformLibrary\Timer.h"
//#include "PlatformLibrary\Sys.h"
//#include <functional>
//#include "Common.h"

#include <BaseLibrary\Timer.hpp>
#include <algorithm>

using namespace inl;

VisualCpuProfiler* VisualCpuProfiler::instance = nullptr;
VisualCpuProfiler::ProfilerNode* VisualCpuProfiler::lastConstructedTreeNode = nullptr;
size_t VisualCpuProfiler::IDGenerator = 0;

// Scope profiling
VisualCpuProfiler::Scope::Scope(const std::string& name)
:name(name)
{
	timer = new Timer();
	
	// Add that Section to tree
	ProfilerNode* node = new ProfilerNode();
		node->ID = IDGenerator;
		node->name = name;
	
	// This is a root node !
	if (lastConstructedTreeNode == nullptr)
	{
		node->parent = nullptr;

		auto& treeComps = VisualCpuProfiler::GetSingletonInstance()->treeRootComponents;

		// Search with ID
		auto it = std::find_if(treeComps.begin(), treeComps.end(), [&](ProfilerNode* node) -> bool
		{
			return node->ID == IDGenerator;/* && node->levelOfDeepness == levelOfDeepness*/;
		});

		if (it == treeComps.end())
			VisualCpuProfiler::GetSingletonInstance()->treeRootComponents.push_back(node);
		else
		{
			ProfilerNode* tmp = (*it);

			auto savedChilds = tmp->childs;

			if (!tmp->bSelfDelete)
				delete tmp;

			(*it) = node;
			(*it)->childs = savedChilds;
		}
	}
	else // This is a child node
	{
		node->parent = lastConstructedTreeNode;

		auto& lastConstructedsChilds = lastConstructedTreeNode->childs;

		auto it = std::find_if(lastConstructedsChilds.begin(), lastConstructedsChilds.end(), [&](ProfilerNode* node) -> bool
		{
			return node->ID == IDGenerator;
		});

		if (it == lastConstructedsChilds.end())
		{
			lastConstructedsChilds.push_back(node);
		}
		else
		{
			ProfilerNode* tmp = (*it);

			auto savedChilds = tmp->childs;

			if (!tmp->bSelfDelete)
				delete tmp;

			(*it) = node;
			(*it)->childs = savedChilds;
		}
	}
	
	IDGenerator++;
	lastConstructedTreeNode = node;

	timer->Start();
}

VisualCpuProfiler::Scope::~Scope()
{
	// Save profiled time
	lastConstructedTreeNode->profiledSeconds = timer->Elapsed();
	
	// Go up 1 on tree
	lastConstructedTreeNode = lastConstructedTreeNode->parent;
	
	delete timer;
}

// Scope sum profiling
VisualCpuProfiler::ScopeSum::LifeCycleHelper::LifeCycleHelper(ScopeSum* scopeSumProfiler)
:scopeSumProfiler(scopeSumProfiler)
{
	auto& treeRootComponents = VisualCpuProfiler::GetSingletonInstance()->treeRootComponents;
	if (!lastConstructedTreeNode)
	{
		scopeSumProfiler->profilerNode->parent = nullptr;

		auto& treeComps = VisualCpuProfiler::GetSingletonInstance()->treeRootComponents;

		// Search with ID
		auto it = std::find_if(treeComps.begin(), treeComps.end(), [&](ProfilerNode* node) -> bool
		{
			return node->name == scopeSumProfiler->profilerNode->name;/* && node->levelOfDeepness == levelOfDeepness*/;
		});

		if (it == treeComps.end())
		{
			treeComps.push_back(scopeSumProfiler->profilerNode);
		}
		else
		{
			ProfilerNode* tmp = (*it);

			auto savedChilds = tmp->childs;

			if (!tmp->bSelfDelete)
				delete tmp;

			(*it) = scopeSumProfiler->profilerNode;
			(*it)->childs = savedChilds;
		}
	}
	else// if (scopeSumProfiler->ID == IDGenerator++)
	{
		scopeSumProfiler->profilerNode->parent = lastConstructedTreeNode;

		auto& lastConstructedsChilds = lastConstructedTreeNode->childs;

		auto it = std::find_if(lastConstructedsChilds.begin(), lastConstructedsChilds.end(), [&](ProfilerNode* node) -> bool
		{
			return node->name == scopeSumProfiler->profilerNode->name;
		});

		if (it == lastConstructedsChilds.end())
		{
			scopeSumProfiler->profilerNode->profiledSeconds = 0;
			lastConstructedTreeNode->childs.push_back(scopeSumProfiler->profilerNode);
		}
		else
		{
			ProfilerNode* tmp = (*it);

			auto savedChilds = tmp->childs;

			if (!tmp->bSelfDelete)
				delete tmp;

			(*it) = scopeSumProfiler->profilerNode;
			(*it)->childs = savedChilds;
		}
	}

	lastConstructedTreeNode = scopeSumProfiler->profilerNode;

	scopeSumProfiler->timer->Reset();
}

VisualCpuProfiler::ScopeSum::LifeCycleHelper::~LifeCycleHelper()
{
	scopeSumProfiler->profilerNode->profiledSeconds += scopeSumProfiler->timer->Elapsed();

	// Go up 1 on tree
	lastConstructedTreeNode = lastConstructedTreeNode->parent;
}

VisualCpuProfiler::ScopeSum::ScopeSum(const std::string& name)
:name(name)
{
	// Problematic: DLL_BUILD -> IDGenerator = 0, STATIC_BUILD -> IDGenerator > 0
	ID = IDGenerator;

	timer = new Timer();

	profilerNode = new ProfilerNode();
	profilerNode->bSelfDelete = true;
	profilerNode->name = name;
	profilerNode->ID = IDGenerator;
}

VisualCpuProfiler::ScopeSum::~ScopeSum()
{
	delete profilerNode;
	delete timer;
}

VisualCpuProfiler::VisualCpuProfiler()
{
	//window.create(sf::VideoMode(600, 600), "Engine - CpuProfiler");
	//window.setPosition({ 0, 0 });
	//
	//bool b = fontArial.loadFromFile(GetAssetsDir() + "arial.ttf");
	//assert(b);
}

void VisualCpuProfiler::_internalupdateAndPresent()
{
	IDGenerator = 0;

	//sf::Event evt;
	//while (window.pollEvent(evt))
	//{
	//	if (evt.type == sf::Event::Closed)
	//		window.close();
	//	else if (evt.type == sf::Event::Resized)
	//	{
	//		sf::View view;
	//			view.setCenter(sf::Vector2f((float)evt.size.width * 0.5, (float)evt.size.height * 0.5));
	//			view.setSize(sf::Vector2f((float)evt.size.width, (float)evt.size.height));
	//			view.setViewport(sf::FloatRect(0, 0, 1, 1));
	//		window.setView(view);
	//	}
	//}
	//
	//if (window.isOpen())
	//{
	//	window.setActive(true);
	//
	//	// Unbind everything from opengl
	//	glBindVertexArray(0);
	//	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//	glBindTexture(0, 0);
	//	
	//	glBindVertexBuffers(0, 16, nullptr, nullptr, nullptr);
	//	
	//	for (uint8_t i = 0; i < 16; i++)
	//	{
	//		glBindBufferBase(GL_UNIFORM_BUFFER, i, 0);
	//		glBindVertexBuffer(i, 0, 0, 0);
	//	}
	//	
	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//	glVertexPointer(0, GL_FLOAT, 0, 0);
	//
	//	window.resetGLStates();
	//	window.clear(sf::Color::Black);	
	//
	//	// Search for lowest perf node
	//	double lowestPerfSec = 0;
	//	lowestPerfSectionNode = nullptr;
	//	for (auto& a : treeRootComponents)
	//	{
	//		if (lowestPerfSec <= a->profiledSeconds)
	//		{
	//			lowestPerfSec = a->profiledSeconds;
	//			lowestPerfSectionNode = a;
	//		}
	//	}
	//
	//	size_t curNodePosY = 0;
	//	for (auto& a : treeRootComponents)
	//		_internalDrawSectionTreeRecursively(a, curNodePosY, 0);
	//	
	//	window.display();
	//	window.setActive(false);
	//}
	//
	//// Traverse tree recursivel
	////recursiveJob;
	//static std::function<void(ProfilerNode* a)> recursiveJob = [&](ProfilerNode* a)
	//{
	//	if (a->bSelfDelete)
	//		a->profiledSeconds = 0;
	//
	//	for (auto c : a->childs)
	//		recursiveJob(c);
	//};
	//
	//for (auto a : treeRootComponents)
	//	recursiveJob(a);
}

void VisualCpuProfiler::_internalDrawSectionTreeRecursively(ProfilerNode* node, size_t& curNodePosY_inout, size_t depth)
{
	// Draw that node...
	//sf::Text text;
	//std::stringstream ss;
	//
	//ss.str("");
	//ss << std::setprecision(2);
	//ss << std::fixed;
	//ss << (double)node->profiledSeconds * 1000;
	//
	//std::string printThat = "- " + node->name + ": " + ss.str() + " ms";
	//sf::String str(printThat.c_str());
	//
	//text.setString(str);
	//text.setFont(fontArial);
	//text.setCharacterSize(20);
	//
	//float interp = 0.f;
	//
	//if (lowestPerfSectionNode->profiledSeconds != 0)
	//	interp = (float)((double)node->profiledSeconds / lowestPerfSectionNode->profiledSeconds);
	//
	//sf::Color hybridColor;
	//	hybridColor.r = round(255.f * interp);
	//	hybridColor.g = round(255.f * (1 - interp));
	//	hybridColor.b = 0;
	//	hybridColor.a = 255.f;
	//text.setColor(hybridColor);
	//
	//text.setPosition(20 + 50 * depth, 20 + curNodePosY_inout);
	//window.draw(text);
	//
	//curNodePosY_inout += 30;

	for (auto& n : node->childs)
		_internalDrawSectionTreeRecursively(n, curNodePosY_inout, depth + 1);
}

void VisualCpuProfiler::UpdateAndPresent()
{
	GetSingletonInstance();
	instance->_internalupdateAndPresent();
}

VisualCpuProfiler* VisualCpuProfiler::GetSingletonInstance()
{
	if (!instance)
		instance = new VisualCpuProfiler();

	return instance;
}