#include "JSONParser.h"

#include "GameObject.h"

#include "Application.h"
#include "M_Detour.h"

#include "Component.h"
#include "C_NavMeshAgent.h"
#include "C_Transform.h"


C_NavMeshAgent::C_NavMeshAgent(GameObject* owner) : Component(owner, ComponentType::NAVMESH_AGENT)
{
	radius = 0;
	destinationPoint = float3::zero;
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
	CalculatePath(destinationPoint);

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

bool C_NavMeshAgent::SetDestination(float3 destination)
{
	return false;
}

bool C_NavMeshAgent::HasDestination()
{
	return false;
}

void C_NavMeshAgent::CancelDestination()
{
}

bool C_NavMeshAgent::CalculatePath(float3 destination)
{

	float3 ownerPos = GetOwner()->GetComponent<C_Transform>()->GetWorldPosition();

	int succes = App->detour->calculatePath(ownerPos, destination, areaMask, path);

	return false;
}

