#include "UIElement.h"

//#include "C_Canvas.h" // Not sure if this is needed

UIElement::UIElement(C_Canvas* owner, UIElementType type, bool isActive) :	type(type),	owner(owner), isActive(isActive)
{

}

UIElement::~UIElement()
{

}

bool UIElement::Update()
{
	return true;
}

bool UIElement::CleanUp()
{
	return true;
}

C_Canvas* UIElement::GetOwner() const
{
	return owner;
}