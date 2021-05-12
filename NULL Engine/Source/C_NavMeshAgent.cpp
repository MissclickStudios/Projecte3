#include "JSONParser.h"

#include "GameObject.h"

#include "Application.h"
#include "M_Detour.h"

#include "C_NavMeshAgent.h"
#include "C_Transform.h"


C_NavMeshAgent::C_NavMeshAgent(GameObject* owner) : Component(owner, ComponentType::NAVMESH_AGENT)
{
	areaMask = 0;
	destination = float3::zero;
	path = float3::zero;
	radius = 0;
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

bool C_NavMeshAgent::CalculatePath(float3 originPos, float3 targetPos)
{
	std::vector<float3> path;
	
	int succes = App->detour->calculatePath(originPos, targetPos, areaMask, path);



	
	return false;
}

