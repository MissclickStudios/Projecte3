#include "JSONParser.h"

#include "R_NavMesh.h"

#include "RecastNavigation/Detour/Include/DetourNavMesh.h"

R_NavMesh::R_NavMesh() : Resource(ResourceType::NAVMESH_AGENT)
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

void R_NavMesh::SetNavMeshName(const char* newName)
{
	navMeshName = newName;
}

const char* R_NavMesh::GetNavMeshName()
{
	return navMeshName.c_str();
}
