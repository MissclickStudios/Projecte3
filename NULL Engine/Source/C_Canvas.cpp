#include "GameObject.h"
#include "C_Transform.h"

#include "C_Canvas.h"

#include "OpenGL.h"

C_Canvas::C_Canvas(GameObject* owner) : Component(owner, ComponentType::CANVAS)
{
	pivot = { GetPosition().x, GetPosition().y };
}

C_Canvas::~C_Canvas()
{

}

bool C_Canvas::Update()
{
	bool ret = true;

	//This will be world space only
	if (IsActive())
	{
		GameObject* owner = GetOwner();
		SetPosition(owner->transform->GetWorldPosition());
		
		//glLineWidth(2.0f);

		//glBegin(GL_LINES);

		//glColor4f(1.0f, 0.0f, 0.0f, 1.0f);											// X Axis.
		//glVertex3f(rect.x - rect.w, rect.y + rect.h, rect.z);		glVertex3f(rect.x + rect.w, rect.y + rect.h, rect.z);
		//glVertex3f(rect.x + rect.w, rect.y + rect.h, rect.z);		glVertex3f(rect.x + rect.w, rect.y - rect.h, rect.z);
		//glVertex3f(rect.x + rect.w, rect.y - rect.h, rect.z);		glVertex3f(rect.x - rect.w, rect.y - rect.h, rect.z);
		//glVertex3f(rect.x - rect.w, rect.y - rect.h, rect.z);		glVertex3f(rect.x - rect.w, rect.y + rect.h, rect.z);

		//glEnd();

		//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}

	return ret;
}

bool C_Canvas::CleanUp()
{
	bool ret = true;

	return ret;
}

bool C_Canvas::SaveState(ParsonNode& root) const
{
	bool ret = true;

	return ret;
}

bool C_Canvas::LoadState(ParsonNode& root)
{
	bool ret = true;

	return ret;
}

float3 C_Canvas::GetPosition() const
{
	return { rect.x, rect.y, rect.z };
}

float2 C_Canvas::GetSize() const
{
	return { rect.w, rect.h };
}

Rect C_Canvas::GetRect() const
{
	return rect;
}

void C_Canvas::SetPosition(const float3& position)
{
	this->rect.x = position.x;
	this->rect.y = position.y;
	this->rect.z = position.z;
}

void C_Canvas::SetSize(const float2& size)
{
	this->rect.w = size.x;
	this->rect.h = size.y;
}

void C_Canvas::SetRect(const Rect& rect)
{
	this->rect.x = rect.x;
	this->rect.y = rect.y;
	this->rect.z = rect.z;
	this->rect.w = rect.w;
	this->rect.h = rect.h;
}