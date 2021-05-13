#include "Application.h"
#include "M_Renderer3D.h"

#include "RE_Cuboid.h"

#define STANDARD_LINE_WIDTH	1.0f
#define BASE_LINE_WIDTH		3.0f

RE_Cuboid::RE_Cuboid(float4x4* transform, float lineWidth, Color color, const float3* vertices, Cuboid_Type cuboidType) : Renderer(RendererType::CUBOID, transform, lineWidth, color),
vertices	(vertices),
cuboidType	(cuboidType)
{

}

RE_Cuboid::RE_Cuboid(const float3* vertices, Cuboid_Type cuboidType) : Renderer(RendererType::CUBOID, nullptr),
vertices	(vertices),
cuboidType	(cuboidType)
{
	color		= GetColorByType();
	lineWidth	= GetEdgeWidthByType();

}

RE_Cuboid::~RE_Cuboid()
{

}

bool RE_Cuboid::Render()
{
	return true;
}

bool RE_Cuboid::CleanUp()
{
	return true;
}

Color RE_Cuboid::GetColorByType()
{
	switch (cuboidType)
	{
	case Cuboid_Type::NONE:		{ return White; }								break;
	case Cuboid_Type::AABB:		{ return App->renderer->GetAABBColor(); }		break;
	case Cuboid_Type::OBB:		{ return App->renderer->GetOBBColor(); }		break;
	case Cuboid_Type::FRUSTUM:	{ return App->renderer->GetFrustumColor(); }	break;
	case Cuboid_Type::COLLIDER:	{ return Green; }								break;
	}

	return White;
}

float RE_Cuboid::GetEdgeWidthByType()
{
	switch (cuboidType)
	{
	case Cuboid_Type::NONE:		{ return BASE_LINE_WIDTH; }							break;
	case Cuboid_Type::AABB:		{ return App->renderer->GetAABBEdgeWidth(); }		break;
	case Cuboid_Type::OBB:		{ return App->renderer->GetOBBEdgeWidth(); }		break;
	case Cuboid_Type::FRUSTUM:	{ return App->renderer->GetFrustumEdgeWidth(); }	break;
	case Cuboid_Type::COLLIDER:	{ return App->renderer->GetOBBEdgeWidth(); }		break;
	}

	return STANDARD_LINE_WIDTH;
}