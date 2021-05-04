#include "C_UI.h"

C_UI::C_UI(GameObject* owner, ComponentType type, bool iInteractuable, Rect2D iRect): Component(owner, type), rect(iRect), interactuable(iInteractuable)
{
}

C_UI::~C_UI()
{
}

bool C_UI::Interactuable() const
{
	return interactuable;
}

Rect2D C_UI::GetRect() const
{
	return rect;
}

void C_UI::SetRect(const Rect2D& rect)
{
	this->rect = rect;
}

void C_UI::SetX(const float x)
{
	this->rect.x = x;
}

void C_UI::SetY(const float y)
{
	this->rect.y = y;
}

void C_UI::SetW(const float w)
{
	this->rect.w = w;
}

void C_UI::SetH(const float h)
{
	this->rect.h = h;
}
