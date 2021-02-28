#include "UIElement.h"

#include "C_Canvas.h" // Not sure if this is needed

UIElement::UIElement(C_Canvas* canvas, UIElementType type, bool isActive) :	type(type),	canvas(canvas), isActive(isActive), isDraggable(false)
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

C_Canvas* UIElement::GetCanvas() const
{
	return canvas;
}

void UIElement::SetCanvas(C_Canvas* canvas)
{
	this->canvas = canvas;
}