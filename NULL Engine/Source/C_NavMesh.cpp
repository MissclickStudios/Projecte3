#include "JSONParser.h"

#include "GameObject.h"

#include "C_NavMesh.h"

C_NavMesh::C_NavMesh(GameObject* owner) : Component(owner, ComponentType::NAVMESH), rNavMesh(nullptr)
{

}

C_NavMesh::~C_NavMesh()
{

}

bool C_NavMesh::Start()
{
	return true;
}

bool C_NavMesh::Update()
{
	return true;
}

bool C_NavMesh::CleanUp()
{
	return true;
}

bool C_NavMesh::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (double)GetType());
	
	return true;
}

bool C_NavMesh::LoadState(ParsonNode& root)
{
	return true;
}