#include "Collectable.h"

#include "GameObject.h"
#include "C_AudioSource.h"

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
	buyAudio = gameObject->GetComponent<C_AudioSource>();
}

void Collectable::Update()
{
	if (used)
	{
		used = false;
		Deactivate();
	}
}

void Collectable::CleanUp()
{
}

void Collectable::OnPause()
{
	CollectablePause();
}

void Collectable::OnResume()
{
	CollectableResume();
}

void Collectable::OnCollisionEnter(GameObject* object)
{
	Player* player = (Player*)object->GetScript("Player");
	if (!player)
		return;

	if (player->credits >= price)
	{
		player->SubtractCredits(price);

		used = true;
		Contact(player);

		if (price != 0 && buyAudio)
			buyAudio->PlayFx(buyAudio->GetEventId());
	}
}
