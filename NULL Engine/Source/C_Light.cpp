#include "JSONParser.h"

#include "GameObject.h"

#include "C_Light.h"

#include "Light.h"

#include "MemoryManager.h"

C_Light::C_Light(GameObject* owner) : Component(owner, ComponentType::LIGHT),
light (new Light())
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
	root.SetFloat4("Diffuse", (math::float4)&light->diffuse);
	root.SetFloat4("Ambient", (math::float4)&light->ambient);

	return ret;
}

bool C_Light::LoadState(ParsonNode& root)
{
	bool ret = true;

	light->Active(true);

	light->diffuse.Set((Color&)root.GetFloat4("Diffuse"));
	light->ambient.Set((Color&)root.GetFloat4("Ambient"));
	light->Init();

	return ret;
}