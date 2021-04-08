#include "R_NavMesh.h"

R_NavMesh::R_NavMesh() : Resource(ResourceType::NAVMESH)
{

}

R_NavMesh::~R_NavMesh()
{

}

bool R_NavMesh::CleanUp()
{
	return true;
}

bool R_NavMesh::SaveMeta(ParsonNode& root) const
{
	return true;
}

bool R_NavMesh::LoadMeta(const ParsonNode& root)
{
	return true;
}