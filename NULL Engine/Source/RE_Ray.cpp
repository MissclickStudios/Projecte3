#include "Application.h"
#include "M_Renderer3D.h"

#include "RE_Ray.h"

RE_Ray::RE_Ray(float4x4* transform, float lineWidth, Color color, const LineSegment& ray) : Renderer(RendererType::RAY, transform, lineWidth, color),
ray (ray)
{

}

RE_Ray::RE_Ray(const LineSegment& ray) : Renderer(RendererType::RAY, nullptr, App->renderer->GetRayWidth(), App->renderer->GetRayColor()),
ray (ray)
{

}

RE_Ray::~RE_Ray()
{

}

bool RE_Ray::Render()
{
	return true;
}

bool RE_Ray::CleanUp()
{
	return true;
}