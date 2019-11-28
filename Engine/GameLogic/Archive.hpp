#pragma once

#include "VariantArchive.hpp"

#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/cereal.hpp>

namespace inl::game {

using InputArchive = VariantInputArchive<cereal::JSONInputArchive, cereal::PortableBinaryInputArchive>;
using OutputArchive = VariantOutputArchive<cereal::JSONOutputArchive, cereal::PortableBinaryOutputArchive>;

} // namespace inl::game
