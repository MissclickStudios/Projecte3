#include "GameObject.h"

#include "WeaponPerk.h"
#include "Player.h"

WeaponPerk::WeaponPerk()
{
}

WeaponPerk::~WeaponPerk()
{
}

void WeaponPerk::Update()
{
	if (used)
	{
		used = false;

		for (uint i = 0; i < gameObject->components.size(); ++i)
			gameObject->components[i]->SetIsActive(false);
		gameObject->SetIsActive(false);
	}
}

void WeaponPerk::CleanUp()
{
}

void WeaponPerk::OnCollisionEnter(GameObject* object)
{
	Player* player = ((Player*)object->GetScript("Player"));
	if (player->coins >= cost)
	{
		player->coins -= cost;
		if (effect == 1)
			player->BlasterStrongShots();
		else
			player->SniperFreezingShots();

		used = true;
	}
}