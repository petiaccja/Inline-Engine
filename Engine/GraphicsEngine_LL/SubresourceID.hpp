#pragma once

#include "../GraphicsApi_LL/IResource.hpp"

namespace inl {
namespace gxeng {

struct SubresourceID {
	SubresourceID() = default;
	SubresourceID(gxapi::IResource* resource, unsigned subresource) : resource(resource), subresource(subresource) {}

	bool operator==(const SubresourceID& other) const {
		return resource == other.resource && subresource == other.subresource;
	}

	gxapi::IResource* resource;
	unsigned subresource;
};

}
}


namespace std {
using namespace inl;
template<>
struct hash<gxeng::SubresourceID> {
	std::size_t operator()(const gxeng::SubresourceID& instance) const {
		return std::hash<gxapi::IResource*>{}(instance.resource) ^ std::hash<unsigned>{}(instance.subresource);
	}
};
}