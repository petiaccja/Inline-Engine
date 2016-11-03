#include "DEBUG_TexturedEntity.hpp"
#include "MemoryObject.hpp"

namespace inl {
namespace gxeng {


void DEBUG_TexturedEntity::SetTexture(Texture2DSRV texture) {
	m_texture = texture;
}


Texture2DSRV DEBUG_TexturedEntity::GetTexture() const {
	return m_texture;
}


} // namespace gxeng
} // namespace inl
