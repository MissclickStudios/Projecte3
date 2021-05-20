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
	GLfloat A[3]	= {};
	GLfloat B[3]	= {};
	uint bytes		= sizeof(float) * 3;

	glColor4f(color.r, color.g, color.b, color.a);
	glLineWidth(lineWidth);
	glBegin(GL_LINES);

	for (uint i = 0; i < bones.size(); ++i)
	{
		memcpy(A, (const void*)bones[i].a.ptr(), bytes);
		memcpy(B, (const void*)bones[i].b.ptr(), bytes);

		glVertex3fv(A);
		glVertex3fv(B);
	}

	glEnd();
	glLineWidth(STANDARD_LINE_WIDTH);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	return true;
}

bool RE_Skeleton::CleanUp()
{
	return true;
}