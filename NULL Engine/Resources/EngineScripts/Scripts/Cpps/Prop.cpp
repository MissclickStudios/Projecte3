#include "Application.h"
#include "Log.h"

#include "GameObject.h"

#include "Prop.h"

Prop::Prop() : Script()
{
}

Prop::~Prop()
{
}

void Prop::Update()
{
	if (dead)
	{
		dead = false;

		for (uint i = 0; i < gameObject->components.size(); ++i)
			gameObject->components[i]->SetIsActive(false);
		gameObject->SetIsActive(false);
	}
}

void Prop::CleanUp()
{
}

void Prop::OnCollisionEnter()
{
	dead = true;
}
