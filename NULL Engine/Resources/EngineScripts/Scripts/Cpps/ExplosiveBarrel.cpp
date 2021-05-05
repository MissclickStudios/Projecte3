#include "Application.h"
#include "M_Scene.h"
#include "Log.h"
#include "GameManager.h"
#include "GameObject.h"

#include "ExplosiveBarrel.h"

ExplosiveBarrel::ExplosiveBarrel() : Script()
{
}

ExplosiveBarrel::~ExplosiveBarrel()
{
}

void ExplosiveBarrel::Start()
{
	gameManager = App->scene->GetGameObjectByName(gameManagerName.c_str());
	explosionObject = gameObject->FindChild(explosionObjectName.c_str());
}

void ExplosiveBarrel::Update()
{
	if (toExplode)
	{
		//Activate explosion collider

		
	}

}

void ExplosiveBarrel::CleanUp()
{
}

void ExplosiveBarrel::OnCollisionEnter(GameObject* object)
{
	if (GetObjectScript(object, ObjectType::BULLET) != nullptr)
	{

	}
}
