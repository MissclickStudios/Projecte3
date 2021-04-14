#include "GameObject.h"
#include "C_Transform.h"
#include "C_Mesh.h"

#include "Coin.h"
#include "Player.h"

#include "MathGeoLib/include/Math/float3.h"

Coin::Coin()
{
	spinTimer.Stop();
}

Coin::~Coin()
{
}

void Coin::Awake()
{
}

void Coin::Update()
{
	if (!mesh)
		for (uint i = 0; i < gameObject->childs.size(); ++i)
			if (gameObject->childs[i]->GetComponent<C_Mesh>())
				mesh = gameObject->childs[i];

	if (!spinTimer.IsActive())
	{
		spinTimer.Start();
		if (mesh)
		{
			float2 currentDir(direction.x, direction.y);
			float currentRad = currentDir.AimedAngle();
			currentRad += 0.05;
			direction.x = cos(currentRad);
			direction.y = sin(currentRad);

			mesh->transform->SetLocalRotation(float3(DegToRad(-90), 0, currentRad));
		}
	}
	else if (spinTimer.ReadSec() >= spinVelocity)
		spinTimer.Stop();

	if (used)
	{
		used = false;

		for (uint i = 0; i < gameObject->components.size(); ++i)
			gameObject->components[i]->SetIsActive(false);
		gameObject->SetIsActive(false);
	}
}

void Coin::CleanUp()
{
}

void Coin::OnCollisionEnter(GameObject* object)
{
	((Player*)object->GetScript("Player"))->coins += value;
	used = true;
}
