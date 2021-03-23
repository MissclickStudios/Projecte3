#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "Color.h"
#include "MathGeoTransform.h"
//#include "MathGeoLib/include/Math/float3.h"

enum class LightType
{
	NONE,
	DIRECTIONAL,
	SPOTLIGHT,
	POINTLIGHT
};

class Light
{
public:

	Light(LightType type);
	~Light();

	virtual void Init();
	virtual void SetPos(float x, float y, float z);
	virtual void Active(bool isActive);
	virtual void Render();

	virtual inline LightType GetType() const { return type; }

public:
	Color ambient;
	Color diffuse;
	Color specular;

	LightType type;

	int ref;
	bool on;
};

#endif // !__LIGHT_H__