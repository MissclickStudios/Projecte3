#include "JSONParser.h"

#include "GameObject.h"

#include "C_Light.h"
#include "C_Transform.h"

#include "Light.h"
#include "Color.h"
#include "MemoryManager.h"

C_Light::C_Light(GameObject* owner, LightType lightType) : Component(owner, ComponentType::LIGHT),
lightType(lightType),
directional(nullptr)
{
	switch (lightType)
	{
	case LightType::DIRECTIONAL: directional = new DirectionalLight(); break;
	case LightType::POINTLIGHT: break;
	case LightType::SPOTLIGHT: break;
	case LightType::NONE: break;
	}
}

C_Light::~C_Light()
{

}

bool C_Light::Update()
{
	bool ret = true;

	directional->SetDirection(float3(-0.2f, -1.0f, -0.3f));

	return ret;
}

bool C_Light::CleanUp()
{
	bool ret = true;

	directional = nullptr;
	
	return ret;
}

bool C_Light::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (uint)GetType());

	if (directional)
	{
		root.SetFloat4("Diffuse", (math::float4) & directional->diffuse);
		root.SetFloat4("Ambient", (math::float4) & directional->ambient);
		root.SetFloat4("Specular", (math::float4) & directional->specular);
		root.SetFloat3("Direction", directional->GetDirection());
	}

	return ret;
}

bool C_Light::LoadState(ParsonNode& root)
{
	bool ret = true;
	if (directional)
	{
		directional->Active(true);
		directional->diffuse.Set((Color&)root.GetFloat4("Diffuse"));
		directional->ambient.Set((Color&)root.GetFloat4("Ambient"));
		directional->specular.Set((Color&)root.GetFloat4("Specular"));
		directional->Init();
	}

	return ret;
}
