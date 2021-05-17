#include "Renderer.h"
#include "Dependencies/glew/include/glew.h"

Renderer::Renderer(RendererType type, float4x4* transform, float lineWidth, Color color) : type(type), transform(transform), lineWidth(lineWidth), color(color)
{

}

Renderer::~Renderer()
{

}

bool Renderer::Render()
{
	return true;
}

bool Renderer::CleanUp()
{
	return true;
}