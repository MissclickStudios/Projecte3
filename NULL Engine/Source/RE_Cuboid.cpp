#include "Application.h"
#include "M_Renderer3D.h"

#include "RE_Cuboid.h"

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
		// For a Cuboid with vertices ABCDEFGH
	GLfloat* A = (GLfloat*)&vertices[0];
	GLfloat* B = (GLfloat*)&vertices[1];
	GLfloat* C = (GLfloat*)&vertices[2];
	GLfloat* D = (GLfloat*)&vertices[3];
	GLfloat* E = (GLfloat*)&vertices[4];
	GLfloat* F = (GLfloat*)&vertices[5];
	GLfloat* G = (GLfloat*)&vertices[6];
	GLfloat* H = (GLfloat*)&vertices[7];

	glColor4f(color.r, color.g, color.b, color.a);
	glLineWidth(lineWidth);
	glBegin(GL_LINES);

	// --- FRONT
	glVertex3fv(A);											// BOTTOM HORIZONTAL										// Firstly the Near Plane is constructed.
	glVertex3fv(B);											// -----------------
	glVertex3fv(D);											// TOP HORIZONTAL
	glVertex3fv(C);											// -------------

	glVertex3fv(B);											// LEFT VERTICAL
	glVertex3fv(D);											// -------------
	glVertex3fv(C);											// RIGHT VERTICAL
	glVertex3fv(A);											// --------------

	// --- BACK
	glVertex3fv(F);											// BOTTOM HORIZONTAL										// Secondly the Far Plane is constructed.
	glVertex3fv(E);											// -----------------
	glVertex3fv(G);											// TOP HORIZONTAL
	glVertex3fv(H);											// -------------- 

	glVertex3fv(E);											// LEFT VERTICAL 
	glVertex3fv(G);											// ------------- 
	glVertex3fv(H);											// RIGHT VERTICAL 
	glVertex3fv(F);											// -------------- 

	// --- RIGHT
	glVertex3fv(F);											// BOTTOM HORIZONTAL										// Lastly, the Near and Far Planes' corners are connected.
	glVertex3fv(B); 										// -----------------
	glVertex3fv(H); 										// TOP HORIZONTAL 
	glVertex3fv(D); 										// -------------- 
	
	// --- LEFT
	glVertex3fv(E);											// BOTTOM HORIZONTAL										// ---
	glVertex3fv(A);											// -----------------
	glVertex3fv(C);											// TOP HORIZONTAL 
	glVertex3fv(G);											// -------------- 

	glEnd();
	glLineWidth(STANDARD_LINE_WIDTH);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	return true;
}

bool RE_Cuboid::CleanUp()
{
	return true;
}

// --- RE_CUBOID MERHODS ---
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