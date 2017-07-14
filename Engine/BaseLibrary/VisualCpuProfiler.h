#pragma once

#include <BaseLibrary\Timer.hpp>

#include <assert.h>
#include <string>
#include <vector>

namespace inl
{

#ifdef PROFILE_ENGINE
	#define CONCAT_MACRO_BASE(x, y)		x##y
	#define CONCAT_MACRO(x,y)			CONCAT_MACRO_BASE(x,y)

	#define PROFILE_SCOPE(strName) \
		VisualCpuProfiler::Scope CONCAT_MACRO(section_, __COUNTER__)(strName)

#define PROFILE_SCOPE_SUM_HELPER_CONCAT(strName, count) \
	static VisualCpuProfiler::ScopeSum CONCAT_MACRO(_node, count)(strName); \
	VisualCpuProfiler::ScopeSum::LifeCycleHelper CONCAT_MACRO(_helper, count)(&CONCAT_MACRO(_node, count));

	#define PROFILE_SCOPE_SUM(strName) \
		PROFILE_SCOPE_SUM_HELPER_CONCAT(strName, __COUNTER__)
#else
	#define PROFILE_SCOPE(x)	 (x)
	#define PROFILE_SCOPE_SUM(x) (x)
#endif

class Timer;

class VisualCpuProfiler
{
public:
	struct ProfilerNode
	{
		ProfilerNode() :name("INVALID"), ID(-1), profiledSeconds(0), parent(0), bSelfDelete(false){}

		std::string name;
		int64_t ID;

		double profiledSeconds;
		std::vector<ProfilerNode*> childs;
		ProfilerNode* parent;

		// TODO spec
		bool bSelfDelete;
	};

	class Scope
	{
	public:
		Scope(const std::string& name);
		~Scope();

	protected:
		Timer* timer;
		std::string name;
	};

	class ScopeSum
	{
	public:
		class LifeCycleHelper
		{
		public:
			LifeCycleHelper(ScopeSum* scopeSumProfiler);
			~LifeCycleHelper();
		protected:
			ScopeSum* scopeSumProfiler;
		};

		friend class LifeCycleHelper;
		ScopeSum(const std::string& name);
		~ScopeSum();

	protected:
		std::string name;
		Timer* timer;
		ProfilerNode* profilerNode;
		size_t ID;
	};

protected:
	VisualCpuProfiler();
	void _internalupdateAndPresent();
	void _internalDrawSectionTreeRecursively(ProfilerNode* node, size_t& curNodePosY_inout, size_t depth);

public:
	static void init();
	static void UpdateAndPresent();
	static VisualCpuProfiler* GetSingletonInstance();

protected:
	static VisualCpuProfiler* instance;
	static VisualCpuProfiler::ProfilerNode* lastConstructedTreeNode;
	static size_t IDGenerator;

	//sf::RenderWindow window;
	//sf::Font fontArial;

public:
	// Profiler node tree
	std::vector<ProfilerNode*> treeRootComponents;
	ProfilerNode* lowestPerfSectionNode;
};

}