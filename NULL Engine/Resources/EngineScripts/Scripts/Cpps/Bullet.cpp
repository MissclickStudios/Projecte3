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
		{
			Player* player = (Player*)shooter->GetScript("Player");
			if(player->weaponUsed == 1)
				player->blaster->DisableProjectile(index);
			else
				player->sniper->DisableProjectile(index);
		}
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
		{
			if(freeze)
				((Blurrg*)script)->Freeze(0.3f, 4.0f);
			if (strong)
				((Blurrg*)script)->Weaken(2.0f, 0.1f);
			((Blurrg*)script)->TakeDamage(damage/2);
		}
		else
		{
			script = object->GetScript("SandTrooper");
			if (script)
			{
				if (freeze)
					((SandTrooper*)script)->Freeze(0.3f, 4.0f);
				if (strong)
					((SandTrooper*)script)->Weaken(2.0f, 0.1f);
				((SandTrooper*)script)->TakeDamage(damage/2);
			}
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

	INSPECTOR_CHECKBOX_BOOL(script->freeze);
	INSPECTOR_CHECKBOX_BOOL(script->strong);

	return script;
}