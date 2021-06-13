#ifndef __C_LIGHT_H__
#define __C_LIGHT_H__

#include "Component.h"
#include "DirectionalLight.h"
#include "PointLight.h"

class ParsonNode;
class GameObject;

class MISSCLICK_API C_Light : public Component
{
public:
	C_Light(GameObject* owner, LightType lightType = LightType::NONE);
	~C_Light();

	bool Update		() override;
	bool CleanUp	() override;

	bool SaveState	(ParsonNode& root) const override;
	bool LoadState	(ParsonNode& root) override;

	inline LightType GetLightType() const { return lightType; }

	inline DirectionalLight* GetDirectionalLight() { if(directional) return directional; }

	inline PointLight* GetPointLight() { if (pointLight) return pointLight; }

	static inline ComponentType GetType() { return ComponentType::LIGHT; }			// This is needed to be able to use templeates for functions such as GetComponent<>();

private:
	LightType lightType;

	DirectionalLight* directional;

	PointLight* pointLight;
};

#endif // !__C_LIGHT_H__