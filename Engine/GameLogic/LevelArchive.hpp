#include "Archive.hpp"

#include <BaseLibrary/DynamicTuple.hpp>


namespace inl::game {


template <class Archive>
class LevelArchive : public Archive {
public:
	template <class T>
	const T* GetModule() const;

	template <class T>
	void RegisterModule(const T* module);

private:
	DynamicTuple m_modules;
};


template <class Archive>
template <class T>
const T* LevelArchive<Archive>::GetModule() const {
	return m_modules.Get<const T*>();
}

template <class Archive>
template <class T>
void LevelArchive<Archive>::RegisterModule(const T* module) {
	m_modules.Insert(module);
}


using LevelInputArchive = LevelArchive<InputArchive>;
using LevelOutputArchive = LevelArchive<OutputArchive>;


} // namespace inl::game