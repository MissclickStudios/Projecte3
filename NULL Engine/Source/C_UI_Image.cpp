#include "Application.h"

#include "GameObject.h"

#include "M_Camera3D.h"
#include "M_Editor.h"
#include "M_Scene.h"
#include "M_UISystem.h"

#include "C_Material.h"
#include "C_Canvas.h"
#include "C_Transform.h"
#include "C_Camera.h"
#include "C_2DAnimator.h"

#include "E_Viewport.h"

#include "C_UI_Image.h"

#include "OpenGL.h"

#include "MemoryManager.h"

#include "JSONParser.h"

C_UI_Image::C_UI_Image(GameObject* owner, Rect2D rect) : Component(owner, ComponentType::UI_IMAGE)
{
	
}

C_UI_Image::~C_UI_Image()
{

}

bool C_UI_Image::Update()
{
	bool ret = true;

	if (IsActive() == false)
		return ret;

	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (canvas == nullptr)
		return ret;

	if (GetRect().w > canvas->GetRect().w)
		SetW(canvas->GetRect().w);

	if(GetRect().h > canvas->GetRect().h)
		SetH(canvas->GetRect().h);

	return ret;
}

bool C_UI_Image::CleanUp()
{
	bool ret = true;

	return ret;
}

void C_UI_Image::Draw2D()
{
	if (GetOwner()->GetComponent<C_Material>() == nullptr && GetOwner()->GetComponent<C_2DAnimator>() == nullptr) return;

	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (canvas == nullptr) return;

	glPushMatrix();
	glMultMatrixf((GLfloat*)&GetOwner()->parent->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	uint32 id;

	if (GetOwner()->GetComponent<C_2DAnimator>() != nullptr && GetOwner()->GetComponent<C_2DAnimator>()->IsAnimationPlaying())
		id = GetOwner()->GetComponent<C_2DAnimator>()->GetIdFromAnimation();
	else
		id = GetOwner()->GetComponent<C_Material>()->GetTextureID();

	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(canvas->GetPosition().x + GetRect().x - GetRect().w / 2, canvas->GetPosition().y + GetRect().y - GetRect().h / 2);
	glTexCoord2f(1, 0); glVertex2f(canvas->GetPosition().x + GetRect().x + GetRect().w / 2, canvas->GetPosition().y + GetRect().y - GetRect().h / 2);
	glTexCoord2f(1, 1); glVertex2f(canvas->GetPosition().x + GetRect().x + GetRect().w / 2, canvas->GetPosition().y + GetRect().y + GetRect().h / 2);
	glTexCoord2f(0, 1); glVertex2f(canvas->GetPosition().x + GetRect().x - GetRect().w / 2, canvas->GetPosition().y + GetRect().y + GetRect().h / 2);
	glEnd();

	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);

}

void C_UI_Image::Draw3D()
{
	if (GetOwner()->GetComponent<C_Material>() == nullptr && GetOwner()->GetComponent<C_2DAnimator>() == nullptr) return;
	
	glPushMatrix();
	glMultMatrixf((GLfloat*)&GetOwner()->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

	glEnable(GL_BLEND);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	uint32 id;

	if (GetOwner()->GetComponent<C_2DAnimator>() != nullptr && GetOwner()->GetComponent<C_2DAnimator>()->IsAnimationPlaying())
		id = GetOwner()->GetComponent<C_2DAnimator>()->GetIdFromAnimation();
	else
		id = GetOwner()->GetComponent<C_Material>()->GetTextureID();

	glBindTexture(GL_TEXTURE_2D, id); // Not sure
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(GetRect().x - GetRect().w / 2, GetRect().y - GetRect().h / 2);
	glTexCoord2f(1, 0); glVertex2f(GetRect().x + GetRect().w / 2, GetRect().y - GetRect().h / 2);
	glTexCoord2f(1, 1); glVertex2f(GetRect().x + GetRect().w / 2, GetRect().y + GetRect().h / 2);
	glTexCoord2f(0, 1); glVertex2f(GetRect().x - GetRect().w / 2, GetRect().y + GetRect().h / 2);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_BLEND);

	glPopMatrix();

}


bool C_UI_Image::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (uint)GetType());

	ParsonNode image = root.SetNode("Image");

	image.SetNumber("X", GetRect().x);
	image.SetNumber("Y", GetRect().y);
	image.SetNumber("W", GetRect().w);
	image.SetNumber("H", GetRect().h);

	if (strcmp(GetOwner()->GetName(), "Hovered Decoration L") == 0)
		image.SetBool("IsHDL", true);
	else
		image.SetBool("IsHDL", false);

	if (strcmp(GetOwner()->GetName(), "Hovered Decoration R") == 0)
		image.SetBool("IsHDR", true);
	else
		image.SetBool("IsHDR", false);

	return ret;
}

bool C_UI_Image::LoadState(ParsonNode& root)
{
	bool ret = true;

	ParsonNode image = root.GetNode("Image");

	Rect2D r;

	r.x = image.GetNumber("X");
	r.y = image.GetNumber("Y");
	r.w = image.GetNumber("W");
	r.h = image.GetNumber("H");

	SetRect(r);

	if (image.GetBool("IsHDL"))
	{
		App->uiSystem->hoveredDecorationL = this;
		App->uiSystem->isHoverDecorationAdded = true;
	}

	if (image.GetBool("IsHDR"))
	{
		App->uiSystem->hoveredDecorationR = this;
		App->uiSystem->isHoverDecorationAdded = true;
	}
	return ret;
}


Rect2D C_UI_Image::GetRect() const
{
	return rect;
}




void C_UI_Image::SetRect(const Rect2D& rect)
{
	this->rect = rect;
}

void C_UI_Image::SetX(const float x)
{
	this->rect.x = x;
}

void C_UI_Image::SetY(const float y)
{
	this->rect.y = y;
}

void C_UI_Image::SetW(const float w)
{
	this->rect.w = w;
}

void C_UI_Image::SetH(const float h)
{
	this->rect.h = h;
}