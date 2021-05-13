#include "Application.h"
#include "M_Renderer3D.h"

#include "RE_Skeleton.h"

RE_Skeleton::RE_Skeleton(float4x4* transform, float lineWidth, Color color, const std::vector<LineSegment>& bones) : Renderer(RendererType::SKELETON, transform, lineWidth, color),
bones	(bones)
{

}

RE_Skeleton::RE_Skeleton(const std::vector<LineSegment>& bones) : Renderer(RendererType::SKELETON, nullptr, App->renderer->GetBoneWidth(), App->renderer->GetBoneColor()),
bones (bones)
{

}

RE_Skeleton::~RE_Skeleton()
{

}

bool RE_Skeleton::Render()
{
	return true;
}

bool RE_Skeleton::CleanUp()
{
	return true;
}