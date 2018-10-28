#include <BaseLibrary/Graph/NodeLibrary.hpp>

extern "C"
extern bool g_autoRegisterSysNodes;
#define INL_SYSNODE_FORCE_REGISTER INL_NODE_FORCE_INCLUDE(g_autoRegisterSysNodes)