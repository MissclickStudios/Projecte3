#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "MathGeoLib/include/Math/float4x4.h"

#include "Color.h"

#define STANDARD_LINE_WIDTH	1.0f
#define BASE_LINE_WIDTH		3.0f

enum class RendererType
{
	MESH,
	CUBOID,
	RAY,
	SKELETON,
	PARTICLE,
	CIRCLE
};

class Renderer
{
public:
	Renderer(RendererType type, float4x4* transform, float lineWidth = 1.0f, Color color = White);
	virtual ~Renderer();

	virtual bool Render();
	virtual bool CleanUp();

protected:
	RendererType	type;
	
	float4x4*		transform;															// Will remain a pointer until the scene is updated hierarchycally.
	float			lineWidth;
	Color			color;
};

#endif // !__RENDERER_H__