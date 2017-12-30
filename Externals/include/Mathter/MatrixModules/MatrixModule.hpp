//==============================================================================
// This software is distributed under The Unlicense. 
// For more information, please refer to <http://unlicense.org/>
//==============================================================================

#pragma once

#include "../DefinitionsUtil.hpp"


// Selective inclusion of matrix module
namespace mathter {
namespace impl {
	template <class T>
	class Empty {};

	template <bool Enable, class Module>
	using MatrixModule = typename std::conditional<Enable, Module, Empty<Module>>::type;
}
}