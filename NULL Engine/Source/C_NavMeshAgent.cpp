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
			nextPoint = path[indexPath];

			direction = nextPoint - currentPos;

			float2 directorVector = direction.xz();

			if (!directorVector.IsZero())
				directorVector.Normalize();

			rigidBody->Set2DVelocity(directorVector * velocity);

			if (GetOwner()->transform->GetDistanceTo(nextPoint) <= 1.0f)
			{
				currentPos = nextPoint;
				nextPoint = path[++indexPath];
			}
			
			if (indexPath == (path.size()-1))
			{
				indexPath = 0;

				directorVector = { 0,0 };

				hasDestination = false;

				rigidBody->Set2DVelocity(directorVector * velocity);
			}
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


	return true;
}

bool C_NavMeshAgent::HasDestination()
{
	return hasDestination;
}

void C_NavMeshAgent::CancelDestination()
{
	hasDestination = false;
	path.clear();
	indexPath = 0;
}

void C_NavMeshAgent::StopAndCancelDestination()
{
	indexPath = 0;
	path.clear();
	hasDestination = false;
	rigidBody->Set2DVelocity({ 0,0 });
}

bool C_NavMeshAgent::AgentPath(float3 origin, float3 destination)
{
	hasDestination = true;
	return  App->detour->CalculatePath(origin, destination, path);
}

