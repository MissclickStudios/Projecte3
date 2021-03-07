#include "GameObject.h"

#include "UIElement.h"

#include "MemoryManager.h"

UIElement::UIElement(GameObject* owner, UIElementType type, Rect rect, bool isActive) :	type(type),	owner(owner), rect(rect), isActive(isActive), isDraggable(false)
{
	GameObject* parent = owner->parent;
	if (parent != nullptr)
	{
		C_Canvas* canvas = parent->GetComponent<C_Canvas>();
		if (canvas != nullptr)
		{
			SetCanvas(canvas);
		}
	}
}

UIElement::~UIElement()
{

}

const char* UIElement::GetNameFromType() const
{
	switch (type)
	{
	case UIElementType::NONE: { return "NONE"; } break;
	case UIElementType::IMAGE: { return "Image"; } break;
	case UIElementType::TEXT: { return "Text"; } break;
	}

	return "NONE";
}

bool UIElement::Update()
{
	return true;
}

bool UIElement::CleanUp()
{
	return true;
}

GameObject* UIElement::GetOwner() const
{
	return owner;
}

C_Canvas* UIElement::GetCanvas() const
{
	return canvas;
}

Rect UIElement::GetRect() const
{
	return rect;
}

float UIElement::GetX() const
{
	return rect.x;
}

float UIElement::GetY() const
{
	return rect.y;
}

float UIElement::GetW() const
{
	return rect.w;
}

float UIElement::GetH() const
{
	return rect.h;
}

bool UIElement::IsActive() const
{
	return isActive;
}


void UIElement::SetOwner(GameObject* owner)
{
	this->owner = owner;
}

void UIElement::SetCanvas(C_Canvas* canvas)
{
	this->canvas = canvas;
}

void UIElement::SetRect(const Rect& rect)
{
	this->rect = rect;
}

void UIElement::SetX(const float x)
{
	this->rect.x = x;
}

void UIElement::SetY(const float y)
{
	this->rect.y = y;
}

void UIElement::SetW(const float w)
{
	this->rect.w = w;
}

void UIElement::SetH(const float h)
{
	this->rect.h = h;
}

void UIElement::SetIsActive(const bool& setTo)
{
	this->isActive = setTo;
}