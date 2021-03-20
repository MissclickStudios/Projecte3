#include "Globals.h"
#include "Light.h"
#include <gl/GL.h>
#include "PointLight.h"

PointLight::PointLight() : Light(LightType::POINTLIGHT)
{
}

PointLight::~PointLight()
{
	glDisable(ref);
}

void PointLight::Init()
{
	glLightfv(ref, GL_AMBIENT, &ambient);
	glLightfv(ref, GL_DIFFUSE, &diffuse);
	glLightfv(ref, GL_SPECULAR, &specular);
	ref = GL_LIGHT0;
}

void PointLight::SetPos(float x, float y, float z)
{
	/*position.x = x;
	position.y = y;
	position.z = z;*/
}

void PointLight::Active(bool isActive)
{
	if (on != isActive)
	{
		on = isActive;

		(on) ? glEnable(ref) : glDisable(ref);
	}
}

void PointLight::Render()
{
	//if (on)
	//{
	//	float pos[] = { position.x, position.y, position.z, 1.0f };
	//	glLightfv(ref, GL_POSITION, pos);
	//}
}
