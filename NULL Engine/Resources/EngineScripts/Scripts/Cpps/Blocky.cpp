#include "Blocky.h"
#include "Log.h"
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
	agent->SetDestination({ -69.0f, 0.0f, -7.0f });
}

void Blocky::PreUpdate()
{
}

void Blocky::Update()
{
	
}

void Blocky::PostUpdate()
{
}

void Blocky::CleanUp()
{

}
