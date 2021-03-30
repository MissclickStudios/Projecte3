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

#include "C_UI_Button.h"

#include "OpenGL.h"

#include "MemoryManager.h"

#include "JSONParser.h"

C_UI_Button::C_UI_Button(GameObject* owner, Rect2D rect) : Component(owner, ComponentType::UI_BUTTON)
{
	if (App->uiSystem->activeButtons.empty())
	{
		state = UIButtonState::HOVERED;
		App->uiSystem->hoveredButton = this;
	}
	App->uiSystem->activeButtons.push_back(this);

	owner->CreateComponent(ComponentType::MATERIAL);// Temp: this is just so buttons have colors when created
}

C_UI_Button::~C_UI_Button()
{

}

bool C_UI_Button::Update()
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

bool C_UI_Button::CleanUp()
{
	bool ret = true;

	return ret;
}

void C_UI_Button::Draw2D()
{
	if (GetOwner()->GetComponent<C_Material>() == nullptr) return;

	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (canvas == nullptr) return;

	glPushMatrix();
	glMultMatrixf((GLfloat*)&GetOwner()->parent->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

	if (state == UIButtonState::HOVERED)
		glColor4f(1.0f, 0.0f, 1.0f, 1.0f);
	else if(state==UIButtonState::PRESSED)
		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
	else
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	uint32 id = GetOwner()->GetComponent<C_Material>()->GetTextureID();
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

void C_UI_Button::Draw3D()
{
	if (GetOwner()->GetComponent<C_Material>() == nullptr) return;
	
	glPushMatrix();
	glMultMatrixf((GLfloat*)&GetOwner()->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

	glEnable(GL_BLEND);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	uint32 id = GetOwner()->GetComponent<C_Material>()->GetTextureID();
	glBindTexture(GL_TEXTURE_2D, id); 
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


bool C_UI_Button::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (uint)GetType());

	ParsonNode button = root.SetNode("Button");

	button.SetNumber("X", GetRect().x);
	button.SetNumber("Y", GetRect().y);
	button.SetNumber("W", GetRect().w);
	button.SetNumber("H", GetRect().h);

	if (state == UIButtonState::HOVERED || state == UIButtonState::PRESSED)
		button.SetBool("IsHovered", true);
	else
		button.SetBool("IsHovered", false);

	return ret;
}

bool C_UI_Button::LoadState(ParsonNode& root)
{
	bool ret = true;

	ParsonNode button = root.GetNode("Button");

	Rect2D r;

	r.x = button.GetNumber("X");
	r.y = button.GetNumber("Y");
	r.w = button.GetNumber("W");
	r.h = button.GetNumber("H");

	SetRect(r);

	if (button.GetBool("IsHovered"))
		state = UIButtonState::HOVERED;
	else
		state = UIButtonState::IDLE;

	return ret;
}


Rect2D C_UI_Button::GetRect() const
{
	return rect;
}

UIButtonState C_UI_Button::GetState() const
{
	return state;
}



void C_UI_Button::SetRect(const Rect2D& rect)
{
	this->rect = rect;
}

void C_UI_Button::SetState(const UIButtonState& setTo)
{
	state = setTo;
}

void C_UI_Button::SetX(const float x)
{
	this->rect.x = x;
}

void C_UI_Button::SetY(const float y)
{
	this->rect.y = y;
}

void C_UI_Button::SetW(const float w)
{
	this->rect.w = w;
}

void C_UI_Button::SetH(const float h)
{
	this->rect.h = h;
}