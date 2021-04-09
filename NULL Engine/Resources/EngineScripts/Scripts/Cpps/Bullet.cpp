#include "Application.h"
#include "Log.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "Bullet.h"
#include "Player.h"
#include "Blurrg.h"
#include "SandTrooper.h"

#include "Weapon.h"

Bullet::Bullet() : Script()
{
	autodestructTimer.Stop();
}

Bullet::~Bullet()
{
}

void Bullet::Update()
{
	if (autodestructTimer.ReadSec() >= autodestruct)
		hit = true;

	if (hit)
	{
		hit = false;

		gameObject->transform->SetWorldPosition(float3::zero);
		for (uint i = 0; i < gameObject->components.size(); ++i)
			gameObject->components[i]->SetIsActive(false);
		gameObject->SetIsActive(false);

		if (target == "enemies")
			((Player*)shooter->GetScript("Player"))->weapon->DisableProjectile(index);
		else if (target == "player")
		{
			((SandTrooper*)shooter->GetScript("SandTrooper"))->weapon->DisableProjectile(index);
		}

		autodestructTimer.Stop();
	}
}

void Bullet::CleanUp()
{
}

void Bullet::OnEnable()
{
	StartAutodestructTimer();
}

void Bullet::OnCollisionEnter(GameObject* object)
{
	hit = true;

	if (target == "enemies")
	{
		void* script = object->GetScript("Blurrg");
		if (script)
			((Blurrg*)script)->TakeDamage(damage);
		else
		{
			script = object->GetScript("SandTrooper");
			if (script)
				((SandTrooper*)script)->TakeDamage(damage);
		}
	}
	else if (target == "player")
	{
		void* script = object->GetScript("Player");
		if (script)
			((Player*)script)->TakeDamage(damage);
	}
}

void Bullet::SetShooter(GameObject* shooter, uint index)
{
	this->shooter = shooter;
	this->index = index;

	if (shooter->GetScript("Player"))
		target = "enemies";
	else if (shooter->GetScript("SandTrooper"))
		target = "player";
}


Bullet* CreateBullet() 
{
	Bullet* script = new Bullet();

	INSPECTOR_DRAGABLE_FLOAT(script->damage);
	INSPECTOR_DRAGABLE_FLOAT(script->autodestruct);

	return script;
}