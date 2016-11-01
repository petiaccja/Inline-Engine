#pragma once

#include "MeshEntity.hpp"
#include "ResourceView.hpp"


namespace inl {
namespace gxeng {


class DEBUG_TexturedEntity : public MeshEntity {
public:

	void SetTexture(Texture2DSRV texture);
	Texture2DSRV GetTexture() const;

private:
	Texture2DSRV m_texture;
};


} // namespace gxeng
} // namespace inl
