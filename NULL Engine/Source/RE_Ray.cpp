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
	GLfloat A[3] = { ray.a.x, ray.a.y , ray.a.z };
	GLfloat B[3] = { ray.b.x, ray.b.y , ray.b.z };

	glColor4f(color.r, color.g, color.b, color.a);

	glLineWidth(lineWidth);
	glBegin(GL_LINES);

	glVertex3fv(A);
	glVertex3fv(B);

	glEnd();
	glLineWidth(STANDARD_LINE_WIDTH);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	return true;
}

bool RE_Ray::CleanUp()
{
	return true;
}