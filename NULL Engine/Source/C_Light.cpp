#include "JSONParser.h"

#include "GameObject.h"

#include "C_Light.h"

C_Light::C_Light(GameObject* owner) : Component(owner, COMPONENT_TYPE::LIGHT)
{

}

C_Light::~C_Light()
{

}

bool C_Light::Update()
{
	bool ret = true;

	return ret;
}

bool C_Light::CleanUp()
{
	bool ret = true;

	return ret;
}

bool C_Light::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (uint)GetType());

	return ret;
}

bool C_Light::LoadState(ParsonNode& root)
{
	bool ret = true;



	return ret;
}