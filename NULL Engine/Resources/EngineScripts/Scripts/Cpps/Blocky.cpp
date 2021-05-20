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
	agent->destinationPoint = { 0,0,0 };
}

void Blocky::Start()
{
	LOG("Start");
}

void Blocky::PreUpdate()
{
	LOG("PreUpdate");
}

void Blocky::Update()
{
	gameObject->transform->SetWorldPosition(*(agent->path.begin()+1));	
}

void Blocky::PostUpdate()
{
	LOG("PostUpdate");
}

void Blocky::CleanUp()
{
	LOG("CleanUp");
}
