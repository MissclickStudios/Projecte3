#include "JSONParser.h"

#include "GameObject.h"

#include "C_NavMeshAgent.h"

C_NavMeshAgent::C_NavMeshAgent(GameObject* owner) : Component(owner, ComponentType::NAVMESH_AGENT)
{

}

C_NavMeshAgent::~C_NavMeshAgent()
{

}

bool C_NavMeshAgent::Start()
{
	return true;
}

bool C_NavMeshAgent::Update()
{
	return true;
}

bool C_NavMeshAgent::CleanUp()
{
	return true;
}

bool C_NavMeshAgent::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (double)GetType());
	
	return true;
}

bool C_NavMeshAgent::LoadState(ParsonNode& root)
{
	return true;
}