#include "Application.h"
#include "Log.h"

#include "GameObject.h"
#include "C_Transform.h"

#include "Bullet.h"
#include "Player.h"

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

		if (shooter)
			shooter->bullets[index]->inUse = false;
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
}

void Bullet::SetShooter(Player* shooter, uint index)
{
	this->shooter = shooter;
	this->index = index;
}


Bullet* CreateBullet() 
{
	Bullet* script = new Bullet();

	INSPECTOR_DRAGABLE_FLOAT(script->damage);
	INSPECTOR_DRAGABLE_FLOAT(script->autodestruct);

	return script;
}