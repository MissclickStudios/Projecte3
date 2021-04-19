#include "JSONParser.h"

#include "GameObject.h"

#include "C_Light.h"
#include "C_Transform.h"

#include "Light.h"
#include "Color.h"
#include "MemoryManager.h"
#include "VariableDefinitions.h"

C_Light::C_Light(GameObject* owner, LightType lightType) : Component(owner, ComponentType::LIGHT),
lightType(lightType),
directional(nullptr),
pointLight(nullptr)
{
	switch (lightType)
	{
	case LightType::DIRECTIONAL: 
		directional = new DirectionalLight(); 
		directional->SetDirection(float3(-0.2f, -1.0f, -0.3f));
		this->GetOwner()->transform->SetLocalEulerRotation(directional->GetDirection());
		break;
	case LightType::POINTLIGHT: 
		pointLight = new PointLight();
		
		break;
	case LightType::SPOTLIGHT: break;
	case LightType::NONE: break;
	}
}

C_Light::~C_Light()
{
	directional = nullptr;
	pointLight = nullptr;
}

bool C_Light::Update()
{
	bool ret = true;

	if(directional) directional->SetDirection(this->GetOwner()->transform->GetLocalEulerRotation()* RADTODEG);

	return ret;
}

bool C_Light::CleanUp()
{
	bool ret = true;

	directional = nullptr;
	pointLight = nullptr;
	
	return ret;
}

bool C_Light::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (double)GetType());
	
	root.SetNumber("LightType", (uint)lightType);

	if (directional)
	{
		root.SetColor("Diffuse", directional->diffuse);
		root.SetColor("Ambient", directional->ambient);
		root.SetColor("Specular", directional->specular);

		root.SetFloat3("Direction", directional->GetDirection());
	}
	if (pointLight)
	{
		root.SetColor("Diffuse", pointLight->diffuse);
		root.SetColor("Ambient", pointLight->ambient);
		root.SetColor("Specular", pointLight->specular);

		root.SetNumber("Constant", pointLight->GetConstant());
		root.SetNumber("Linear", pointLight->GetLinear());
		root.SetNumber("Quadratic", pointLight->GetQuadratic());
		root.SetFloat3("Position", pointLight->GetPosition());
	}

	return ret;
}

bool C_Light::LoadState(ParsonNode& root)
{
	bool ret = true;

	lightType = (LightType)(uint)root.GetNumber("LightType");

	switch (lightType)
	{
	case LightType::DIRECTIONAL:	directional = new DirectionalLight();	break;
	case LightType::POINTLIGHT:		pointLight = new PointLight();			break;
	case LightType::SPOTLIGHT:												break;
	case LightType::NONE:													break;
	}

	if (directional)
	{
		directional->Active(true);
		
		directional->diffuse = root.GetColor("Diffuse");
		directional->ambient = root.GetColor("Ambient");
		directional->specular = root.GetColor("Specular");
		directional->SetDirection(root.GetFloat3("Direction"));

		this->GetOwner()->transform->SetLocalEulerRotation(directional->GetDirection());
		directional->Init();
	}
	if (pointLight)
	{
		pointLight->Active(true);
		
		pointLight->diffuse = root.GetColor("Diffuse");
		pointLight->ambient = root.GetColor("Ambient");
		pointLight->specular = root.GetColor("Specular");
		pointLight->SetConstant(root.GetNumber("Constant"));
		pointLight->SetLinear(root.GetNumber("Linear"));
		pointLight->SetQuadratic(root.GetNumber("Quadratic"));
		pointLight->SetPosition(root.GetFloat3("Position"));
		
		pointLight->Init();
	}

	

	return ret;
}
