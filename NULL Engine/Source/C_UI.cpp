#include "C_UI.h"
#include <string> //material needs it
#include "C_Material.h"
#include "GameObject.h"

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

Frame C_UI::GetTexturePosition(int pixelPosX, int pixelPosY, int pixelWidth, int pixelHeight)
{
	C_Material* cMaterial = GetOwner()->GetComponent<C_Material>();
	if (!cMaterial)
		return { 0, 0, 1, 1 };

	uint32 id = cMaterial->GetTextureID();
	unsigned int spritesheetPixelWidth, spritesheetPixelHeight = 0; cMaterial->GetTextureSize(spritesheetPixelWidth, spritesheetPixelHeight);
	if (!spritesheetPixelWidth && !spritesheetPixelHeight)
		return { 0, 0, 1, 1 };

	Frame frame;
	frame.proportionBeginX = (float)pixelPosX / spritesheetPixelWidth;
	frame.proportionFinalX = ((float)pixelPosX + pixelWidth) / spritesheetPixelWidth;

	frame.proportionBeginY = (float)pixelPosY / spritesheetPixelHeight;
	frame.proportionFinalY = ((float)pixelPosY + pixelHeight) / spritesheetPixelHeight;

	return frame;
}

Frame C_UI::GetTexturePosition(int pixelPosX, int pixelPosY, int pixelWidth, int pixelHeight, int textW, int textH)
{
	Frame frame;
	frame.proportionBeginX = (float)pixelPosX / textW;
	frame.proportionFinalX = ((float)pixelPosX + pixelWidth) / textW;

	frame.proportionBeginY = (float)pixelPosY / textH;
	frame.proportionFinalY = ((float)pixelPosY + pixelHeight) / textH;

	return frame;
}
