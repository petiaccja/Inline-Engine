#pragma once

#include <optional>
#include <string>


namespace inl::tool {

std::optional<std::string> ShowFileOpenDialog();
std::optional<std::string> ShowFileSaveDialog();

} // namespace inl::tool