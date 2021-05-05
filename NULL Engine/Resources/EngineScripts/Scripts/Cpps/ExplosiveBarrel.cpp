#include "Application.h"
#include "M_Scene.h"
#include "Log.h"
#include "GameManager.h"
#include "GameObject.h"

#include "C_BoxCollider.h"

#include "ExplosiveBarrel.h"

ExplosiveBarrel::ExplosiveBarrel() : Object()
{
	baseType = ObjectType::EXPLOSIVE_BARREL;
}

ExplosiveBarrel::~ExplosiveBarrel()
{
}

void ExplosiveBarrel::Start()
{
	gameManager = App->scene->GetGameObjectByName(gameManagerName.c_str());
	explosionObject = gameObject->FindChild(explosionObjectName.c_str());

	barrelCollider = gameObject->GetComponent<C_BoxCollider>();

}

void ExplosiveBarrel::Update()
{
	if (exploded)
	{
		//deactivate mesh, trigger
		barrelCollider->SetIsActive(false);

		exploded = false;
		//play particles
	}

	if (toExplode)
	{
		//Activate explosion collider

		exploded = true;
		toExplode = false;
	}

}

void ExplosiveBarrel::CleanUp()
{
}

void ExplosiveBarrel::OnCollisionEnter(GameObject* object)
{
	if (GetObjectScript(object, ObjectType::BULLET) != nullptr)
	{
		toExplode = true;
		barrelCollider->SetTrigger(true);
		barrelCollider->SetSize(explosionTriggerSize);
	}
}

void ExplosiveBarrel::OnTriggerRepeat(GameObject* object)
{
	Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);

	LOG("Barrel hit entity lol");

	if (!entity)
		return;

	entity->TakeDamage(damage);

}