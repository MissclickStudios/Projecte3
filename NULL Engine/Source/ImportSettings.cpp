#include "JSONParser.h"

#include "ImportSettings.h"

#include "MemoryManager.h"

ImportSettings::ImportSettings()
{
	globalScale		= 1.0f;
	axis				= AxisOrientation::Y_UP;
	ignoreCameras		= false;
	ignoreLights		= false;

	compression			= 0x070A;										// 0x070A = IL_DXT5. DevIL Flag.
	flipX				= false;
	flipY				= false;
	wrapping			= 0x2901;										// 0x2901 = GL_REPEAT. OpenGL Flag.
	filter				= 0x2601;										// 0x2601 = GL_LINEAR. OpenGL Flag.
	generateMipmaps	= true;
	anisotropy			= true;
}

ImportSettings::~ImportSettings()
{

}

bool ImportSettings::Save(ParsonNode& settings) const
{
	bool ret = true;



	return ret;
}

bool ImportSettings::Load(const ParsonNode& settings)
{
	bool ret = true;



	return ret;
}