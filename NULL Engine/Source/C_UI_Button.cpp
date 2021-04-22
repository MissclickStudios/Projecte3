#include "Application.h"

#include "GameObject.h"

#include "M_Camera3D.h"
#include "M_Editor.h"
#include "M_Scene.h"
#include "M_UISystem.h"

#include "C_Material.h"
#include "C_Transform.h"
#include "C_Camera.h"

#include "M_ResourceManager.h"

#include "R_Shader.h"
#include "R_Texture.h"

#include "C_UI_Button.h"

#include "OpenGL.h"

#include "MemoryManager.h"

#include "JSONParser.h"

C_UI_Button::C_UI_Button(GameObject* owner, Rect2D rect) : Component(owner, ComponentType::UI_BUTTON)
{
	state = UIButtonState::IDLE;

	//C_Canvas* canvas = owner->parent->GetComponent<C_Canvas>();
	
	LoadBuffers();
}

C_UI_Button::~C_UI_Button()
{

}

void C_UI_Button::LoadBuffers()
{
	glGenBuffers(1, &VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VAO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(coordsBuffer), coordsBuffer, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool C_UI_Button::Update()
{
	bool ret = true;

	if (IsActive() == false)
		return ret;

	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (canvas == nullptr)
		return ret;

	if (!isInit)
	{
		if (canvas->activeButtons.empty())
		{
			state = UIButtonState::HOVERED;
			canvas->selectedButton = this;
		}
		else if (state == UIButtonState::HOVERED)
		{
			if (canvas->selectedButton != nullptr)
				canvas->selectedButton->SetState(UIButtonState::IDLE);
			canvas->selectedButton = this;
		}

		canvas->activeButtons.push_back(this);

		isInit = true;
	}

	if (GetRect().w > canvas->GetRect().w)
		SetW(canvas->GetRect().w);

	if(GetRect().h > canvas->GetRect().h)
		SetH(canvas->GetRect().h);

	return ret;
}

bool C_UI_Button::CleanUp()
{
	bool ret = true;

	App->uiSystem->DeleteActiveButton(this);
	
	return ret;
}

void C_UI_Button::Draw2D()
{
	if (GetOwner()->GetComponent<C_Material>() == nullptr) return;
	Color tempColor;


	if (state == UIButtonState::HOVERED)
		tempColor = Color(1.0f, 1.0f, 1.0f, 1.0f);

	else if (state == UIButtonState::PRESSED)
		tempColor = Color(1.0f, 0.4f, 0.0f, 1.0f);

	else
		tempColor = Color(1.0f, 1.0f, 0.0f, 1.0f);

	uint32 id = GetOwner()->GetComponent<C_Material>()->GetTextureID();

	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (canvas == nullptr) return;

	glEnable(GL_BLEND);

	if (!GetOwner()->GetComponent<C_Material>()->GetShader())
		GetOwner()->GetComponent<C_Material>()->SetShader(App->resourceManager->GetShader("UIShader"));

	glUseProgram(GetOwner()->GetComponent<C_Material>()->GetShader()->shaderProgramID);

	float x = canvas->GetPosition().x + GetRect().x;
	float y = canvas->GetPosition().y + GetRect().y;

	float4x4 projectionMatrix = float4x4::FromTRS(float3(x, y, 0), Quat::FromEulerXYZ(0, 0, 0), float3(GetRect().w, GetRect().h, 1)).Transposed();

	glBindTexture(GL_TEXTURE_2D, id);

	GetOwner()->GetComponent<C_Material>()->GetShader()->SetUniformMatrix4("projection", projectionMatrix.ptr());
	GetOwner()->GetComponent<C_Material>()->GetShader()->SetUniformVec4f("inColor", (GLfloat*)&tempColor);


	glBindBuffer(GL_ARRAY_BUFFER, VAO);

	glDrawArrays(GL_TRIANGLES, 0, 6);


	glDisable(GL_BLEND);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);

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

void C_UI_Button::OnPressed()
{
	SetState(UIButtonState::PRESSED);
	SetIsPressed(true);
}

void C_UI_Button::OnReleased()
{
	SetState(UIButtonState::HOVERED);
	SetIsPressed(false);
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

bool C_UI_Button::IsPressed() const
{
	return isPressed;
}


void C_UI_Button::SetRect(const Rect2D& rect)
{
	this->rect = rect;
}

void C_UI_Button::SetState(const UIButtonState& setTo)
{
	state = setTo;
}

void C_UI_Button::SetIsPressed(const bool& setTo)
{
	isPressed = setTo;
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