#include "Blocky.h"
#include "Log.h"

#include "Application.h"
#include "M_Scene.h"

#include "GameObject.h"
#include "C_NavMeshAgent.h"
#include "C_Transform.h"

#include "MathGeoLib/include/Math/float3.h"


Blocky::Blocky() : Script()
{
}

Blocky::~Blocky()
{
}

void Blocky::Awake()
{
	LOG("Awake");

	agent = gameObject->GetComponent<C_NavMeshAgent>();

}

void Blocky::Start()
{
	mando = App->scene->GetGameObjectByName("Mandalorian");

	agent->origin = gameObject->GetComponent<C_Transform>()->GetWorldPosition();
	//agent->SetDestination({ -65.0f, 0.0f, -80.0f });
}

void Blocky::PreUpdate()
{
}

void Blocky::Update()
{
	if (mando)
	{
		agent->SetDestination(mando->transform->GetWorldPosition());
	}
}

void Blocky::PostUpdate()
{
}

void Blocky::CleanUp()
{

}
