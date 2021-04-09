#include "Application.h"
#include "Log.h"
#include "Player.h"
#include "GameObject.h"
#include "Bullet.h"

#include "C_Transform.h"

#include "M_Scene.h"

#include "SandTrooper.h"

enum class SandTrooperState
{
	IDLE,
	RUNNING,
	SHOOTING,
	RELOADING,
	NONE
};

SandTrooper::SandTrooper() : Script()
{
	state = SandTrooperState::IDLE;
}

SandTrooper::~SandTrooper()
{
}

void SandTrooper::Start()
{
	//playerTransform = App->scene->GetGameObjectByName("")


	bulletStorage = App->scene->CreateGameObject("Bullets", App->scene->GetSceneRoot());
	for (uint i = 0; i < BULLET_AMOUNT; ++i)
		bullets[i] = CreateProjectile(i);

}

void SandTrooper::Update()
{
}

void SandTrooper::CleanUp()
{
}

void SandTrooper::OnCollisionEnter(GameObject* object)
{

}

Projectile* SandTrooper::CreateProjectile(uint index)
{
	GameObject* bullet = App->scene->InstantiatePrefab(this->bullet.uid, bulletStorage,float3::zero,gameObject->transform->GetLocalRotation());

	char n[10];
	sprintf_s(n, "%d", index);
	std::string num = n;
	std::string name("SandStormBullet" + num);

	bullet->SetName(name.c_str());

	float3 position = float3::zero;
	bullet->transform->SetWorldPosition(position);

	//((Bullet*)bullet->GetComponent<C_Script>()->GetScriptData())->SetShooter(this, index); //Fix this

	for (uint i = 0; i < bullet->components.size(); ++i)
		bullet->components[i]->SetIsActive(false);
	bullet->SetIsActive(false);

	return new Projectile(bullet);
}