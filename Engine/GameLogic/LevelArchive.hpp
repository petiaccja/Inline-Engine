#pragma once


#include "Archive.hpp"


namespace inl::game {



template <class Archive>
class LevelArchive : public Archive, public ModuleArchive {
public:
	using Archive::Archive;
};



using LevelInputArchive = LevelArchive<InputArchive>;
using LevelOutputArchive = LevelArchive<OutputArchive>;


} // namespace inl::game