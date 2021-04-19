#include "Renderer.h"

Renderer::Renderer(float lineWidth, Color color) : lineWidth(lineWidth), color(color)
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