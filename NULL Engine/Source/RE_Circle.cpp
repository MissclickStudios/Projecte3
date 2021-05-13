#include "VariableDefinitions.h"
#include "VariableTypedefs.h"
#include "Macros.h"
#include "Log.h"

#include "RE_Circle.h"

RE_Circle::RE_Circle(float4x4* transform, float radius, uint sectors, float lineWidth, Color color) : Renderer(RendererType::CIRCLE, transform, lineWidth, color),
radius		(radius),
sectors		(sectors),
vertices	((sectors != 0) ? new float[(sectors * 3)] : nullptr)
{
	if (vertices != nullptr)
	{
		memset(vertices, 0, sectors * 3);
		CalculateVertices();
	}
}

RE_Circle::RE_Circle(float4x4* transform, float radius, uint sectors) : Renderer(RendererType::CIRCLE, transform),
radius		(radius),
sectors		(sectors),
vertices	((sectors != 0) ? new float[(sectors * 3)] : nullptr)
{
	if (vertices != nullptr)
	{
		memset(vertices, 0, sectors * 3);
		CalculateVertices();
	}
}

RE_Circle::~RE_Circle()
{
	RELEASE_ARRAY(vertices);
}

bool RE_Circle::Render()
{
	if (radius == 0.0f || sectors == 0)
	{
		LOG("[ERROR] Circle Renderer: Could not Render Circle! Error: Radius or Sectors were equal to 0.");
		return true;
	}

	glLineWidth(lineWidth);
	glColor4f(color.r, color.g, color.b, color.a);
	glBegin(GL_LINE_LOOP);
	
	glPushMatrix();
	glMultMatrixf((GLfloat*)&transform->Transposed());

	for (uint i = 0, j = 0; i < sectors; ++i, j += 3)
	{
		glVertex3f(vertices[j], vertices[j + 1], vertices[j + 2]);
	}

	glPopMatrix();
	
	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(lineWidth);

	return true;
}

bool RE_Circle::CleanUp()
{
	return true;
}

// --- RE_Circle METHODS
void RE_Circle::CalculateVertices()
{
	float sectorStep	= (2 * PI) / sectors;
	float sectorAngle	= 0.0f;

	for (uint i = 0, j = 0; i < sectors; ++i, j +=3)
	{
		sectorAngle	= i * sectorStep;

		/*vertices[j]		= position.x + (sin(sectorAngle) * radius);
		vertices[j + 1] = position.y + 0.0f;
		vertices[j + 2] = position.z + (cos(sectorAngle) * radius);*/

		vertices[j]		= (sin(sectorAngle) * radius);
		vertices[j + 1] = 0.0f;
		vertices[j + 2] = (cos(sectorAngle) * radius);

		LOG("VERTEX %u: { %.3f, %.3f, %.3f }", i, vertices[j], vertices[j + 1], vertices[j + 2]);
	}
}