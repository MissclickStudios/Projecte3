#include "JSONParser.h"
#include "Log.h"

#include "GameObject.h"

#include "Application.h"
#include "M_Detour.h"

#include "Component.h"
#include "C_NavMeshAgent.h"
#include "C_Transform.h"
#include "C_RigidBody.h"



C_NavMeshAgent::C_NavMeshAgent(GameObject* owner) : Component(owner, ComponentType::NAVMESH_AGENT)
{
	radius = 0;
	origin = owner->GetComponent<C_Transform>()->GetWorldPosition();	
	hasDestination = false;
	currentPos = origin;
	direction = float3::zero;
}

C_NavMeshAgent::~C_NavMeshAgent()
{

}

bool C_NavMeshAgent::Start()
{
	rigidBody = GetOwner()->GetComponent<C_RigidBody>();
	return true;
}

bool C_NavMeshAgent::Update()
{
	if (rigidBody)
	{
		if (hasDestination)
		{
			direction = destinationPoint - currentPos;

			float2 directorVector = direction.xz();

			if (!directorVector.IsZero())
				directorVector.Normalize();

			rigidBody->Set2DVelocity(directorVector * velocity);

			nextPoint = path[i];

			if (GetOwner()->transform->GetDistanceTo(currentPos) < 1.0f)
			{
				currentPos = nextPoint;

				nextPoint = path[++i];
			}
			
			if (i == (path.size()-1))
			{
				i = 0;
				hasDestination = false;
			}

			LOG("%u", i);
			LOG("La posicion del angel: X->%f, Y->%f, Z->%f", currentPos.x, currentPos.y, currentPos.z);

		}
	}

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
	float2 pos = { destination.x, destination.z };
	
	destinationPoint = { pos.x, 0, pos.y };
	
	if(hasDestination == false)
		AgentPath(origin, destination);


	return false;
}

bool C_NavMeshAgent::HasDestination()
{
	return hasDestination;
}

void C_NavMeshAgent::CancelDestination()
{
}

bool C_NavMeshAgent::AgentPath(float3 origin, float3 destination)
{
	hasDestination = true;
	return  App->detour->CalculatePath(origin, destination, path);
}

