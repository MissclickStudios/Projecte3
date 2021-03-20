#include "Globals.h"
#include "Light.h"
#include <gl/GL.h>
//#include <gl/GLU.h>

#include "MemoryManager.h"

Light::Light(LightType lightType) :
type		(lightType),
ref			(-1), 
on			(false)
//position	(0.0f, 0.0f, 0.0f)
{

}

Light::~Light()
{
}

void Light::Init()
{
	/*glLightfv(ref, GL_AMBIENT, &ambient);
	glLightfv(ref, GL_DIFFUSE, &diffuse);
	ref = GL_LIGHT0;*/
}

void Light::SetPos(float x, float y, float z)
{
	/*position.x = x;
	position.y = y;
	position.z = z;*/
}

void Light::Render()
{
	/*if(on)
	{
		float pos[] = {position.x, position.y, position.z, 1.0f};
		glLightfv(ref, GL_POSITION, pos);
	}*/
}

void Light::Active(bool isActive)
{
	/*if(on != isActive)
	{
		on = isActive;

		(on) ? glEnable(ref) : glDisable(ref);
	}*/
}