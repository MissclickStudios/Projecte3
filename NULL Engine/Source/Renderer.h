#ifndef __RENDERER_H__
#define __RENDERER_H__

#include "Color.h"

class Renderer
{
public:
	Renderer(float lineWidth = 1.0f, Color color = White);
	virtual ~Renderer();

	virtual bool Render();
	virtual bool CleanUp();

protected:
	float lineWidth;
	Color color;
};

#endif // !__RENDERER_H__