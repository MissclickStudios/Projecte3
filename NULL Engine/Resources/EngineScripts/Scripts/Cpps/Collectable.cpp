#include "Collectable.h"

#include "GameObject.h"

#include "Player.h"

Collectable::Collectable() : Object()
{
	baseType = ObjectType::COLLECTABLE;
}

Collectable::~Collectable()
{
}

void Collectable::Start()
{
}

void Collectable::Update()
{
	SetTypeInt();

	if (used)
	{
		used = false;
		Deactivate();
	}
}

void Collectable::CleanUp()
{
}

void Collectable::OnTriggerEnter(GameObject* object)
{
	Player* player = (Player*)object->GetScript("Player");
	if (!player)
		return;

	used = true;

	if (player->currency >= price)
	{
		player->currency -= price;
		Contact(player);
	}
}
