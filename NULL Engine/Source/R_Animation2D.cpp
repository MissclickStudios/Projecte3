#include "JSONParser.h"

#include "R_Animation2D.h"

R_Animation2D::R_Animation2D() : Resource(ResourceType::ANIMATION2D)
{
}

R_Animation2D::~R_Animation2D()
{
}

bool R_Animation2D::CleanUp()
{
	return true;
}

bool R_Animation2D::SaveMeta(ParsonNode& metaRoot) const
{
	return true;
}

bool R_Animation2D::LoadMeta(const ParsonNode& metaRoot)
{
	return true;
}
