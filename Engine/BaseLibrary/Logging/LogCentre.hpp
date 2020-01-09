#pragma once

#include "../Singleton.hpp"
#include "Logger.hpp"


namespace inl {

/// <summary>
/// System-wide logging facility.
/// See <see cref="Logger"> for more on how it works, it's just a singletonified Logger.
/// </summary>
using LogCentre = Singleton<Logger>;

} // namespace inl