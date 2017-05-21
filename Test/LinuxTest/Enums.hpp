#pragma once

namespace inl::net
{
	enum DistributionType
	{
		All, // Others and Server
		AllAndMe, // Other, Server and the user sending the message
		Server, // just server
		Others, // everyone except server and me
		ID, // specific id
	};
}