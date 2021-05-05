#include "Application.h"
#include "M_Scene.h"
#include "Log.h"
#include "GameManager.h"
#include "GameObject.h"

#include "C_BoxCollider.h"

#include "SarlaacTrap.h"

SarlaacTrap::SarlaacTrap() : Script()
{
}

SarlaacTrap::~SarlaacTrap()
{
}

void SarlaacTrap::Start()
{
	gameManager = App->scene->GetGameObjectByName(gameManagerName.c_str());
	explosionObject = gameObject->FindChild(explosionObjectName.c_str());

	barrelCollider = gameObject->GetComponent<C_BoxCollider>();

}

void SarlaacTrap::Update()
{
	if (exploded)
	{
		//deactivate mesh, trigger
		barrelCollider->SetIsActive(false);
		barrelCollider->SetSize(barrelColliderSize);

		exploded = false;
		//play particles
	}

	if (toExplode)
	{
		//Activate explosion collider

		barrelCollider->SetIsActive(true);

		exploded = true;
		toExplode = false;
	}

}

void SarlaacTrap::CleanUp()
{
}

void SarlaacTrap::OnCollisionEnter(GameObject* object)
{
	if (GetObjectScript(object, ObjectType::BULLET) != nullptr)
	{
		toExplode = true;
		barrelCollider->SetTrigger(true);
		//barrelCollider->SetIsActive(false);
		barrelCollider->SetSize(explosionTriggerSize);
	}
}

void SarlaacTrap::OnTriggerRepeat(GameObject* object)
{
	Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);

	LOG("Barrel hit entity lol");

	if (!entity)
		return;

	entity->TakeDamage(damage);

}