#include "JSONParser.h"

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

bool R_NavMesh::SaveMeta(ParsonNode& metaRoot) const
{
	ParsonArray contained_array = metaRoot.SetArray("ContainedResources");
	
	// REMEMBER TO GENERATE AN ASSET FILE FOR THIS

	return true;
}

bool R_NavMesh::LoadMeta(const ParsonNode& metaRoot)
{
	return true;
}