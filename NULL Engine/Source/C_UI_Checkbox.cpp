#include "C_UI_Checkbox.h"
#include "Dependencies/glew/include/glew.h"
#include "C_Canvas.h"
#include "GameObject.h"
#include "M_Input.h"
#include "Application.h"
#include "C_Material.h"
#include "M_ResourceManager.h"
#include "R_Shader.h"
#include "JSONParser.h"

C_UI_Checkbox::C_UI_Checkbox(GameObject* owner, Rect2D rect) : C_UI(owner, ComponentType::UI_CHECKBOX, true, rect), state(UICheckboxState::UNCHECKED)
{
	LoadBuffers();
}

C_UI_Checkbox::~C_UI_Checkbox()
{
	GameObject* parent = GetOwner()->parent;
	if (parent)
	{
		C_Canvas* canvas = parent->GetComponent<C_Canvas>();
		if (canvas)
			canvas->RemoveUiElement(this);
	}
}

bool C_UI_Checkbox::Update()
{
	return true;
}

bool C_UI_Checkbox::CleanUp()
{
	return true;
}

bool C_UI_Checkbox::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	root.SetNumber("X", rect.x);
	root.SetNumber("Y", rect.y);
	root.SetNumber("W", rect.w);
	root.SetNumber("H", rect.h);

	root.SetInteger("childOrder", childOrder);
	return true;
}

bool C_UI_Checkbox::LoadState(ParsonNode& root)
{
	rect.x = root.GetNumber("X");
	rect.y = root.GetNumber("Y");
	rect.w = root.GetNumber("W");
	rect.h = root.GetNumber("H");

	childOrder = root.GetInteger("childOrder");
	return true;
}

void C_UI_Checkbox::LoadBuffers()
{
	const float coordsBuffer[] = {
		1, 1,
		1, 0,
		0, 0,
		1,0,
	};
	//TODO: spritesheet calculations
	glGenBuffers(1, &VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VAO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(coordsBuffer), coordsBuffer, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void C_UI_Checkbox::HandleInput(C_UI** selectedUi)
{
	if (!IsActive())
		return;

	switch (state)
	{
	case UICheckboxState::NONE:
		state = UICheckboxState::UNCHECKED;
		break;

	case UICheckboxState::UNCHECKED:
		if (*selectedUi == nullptr || *selectedUi == this)
		{
			state = UICheckboxState::HOVEREDUNCHECKED;
			*selectedUi = this;
		}
		break;

	case UICheckboxState::CHECKED:
		if (*selectedUi == nullptr || *selectedUi == this)
		{
			state = UICheckboxState::HOVEREDCHECKED;
			*selectedUi = this;
		}
		break;

	case UICheckboxState::HOVEREDUNCHECKED:
		if (*selectedUi != this)
		{
			state = UICheckboxState::UNCHECKED;
			break;
		}
		if (App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_DOWN) // TODO: make inputs mappable
			state = UICheckboxState::SWAPTOCHECKED;
		break;

	case UICheckboxState::HOVEREDCHECKED:
		if (*selectedUi != this)
		{
			state = UICheckboxState::CHECKED;
			break;
		}
		if (App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_DOWN) // TODO: make inputs mappable
			state = UICheckboxState::SWAPTOUNCHECKED;
		break;

	case UICheckboxState::SWAPTOCHECKED:
		state = UICheckboxState::HOVEREDCHECKED;
		break;

	case UICheckboxState::SWAPTOUNCHECKED:
		state = UICheckboxState::HOVEREDUNCHECKED;
		break;

	default:
		state = UICheckboxState::UNCHECKED;
		break;
	}
}

void C_UI_Checkbox::Draw2D()
{
	/*C_Material* cMaterial = GetOwner()->GetComponent<C_Material>();
	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (cMaterial == nullptr || !cMaterial->GetShader() || canvas == nullptr)
		return;

	//TODO: Inspector pick color !!!
	Color tempColor;
	switch (state)
	{
	case UIButtonState::IDLE:
		tempColor = Color(0.97f, 0.76f, 0.58f, 1.0f); break;
	case UIButtonState::HOVERED:
		tempColor = Color(1.0f, 1.0f, 1.0f, 1.0f); break;
	case UIButtonState::PRESSEDIN:
		tempColor = Color(1.0f, 0.4f, 0.19f, 1.0f); break;
	case UIButtonState::PRESSED:
		tempColor = Color(1.0f, 0.4f, 0.19f, 1.0f); break;
	case UIButtonState::RELEASED:
		tempColor = Color(1.0f, 0.4f, 0.19f, 1.0f); break;
	default:
		tempColor = Color(0.97f, 0.76f, 0.58f, 1.0f); break;
	}
	R_Shader* shader = cMaterial->GetShader();
	uint32 id = cMaterial->GetTextureID();

	Rect2D parentRect = canvas->GetRect();

	glEnable(GL_BLEND);

	if (!shader)
		cMaterial->SetShader(App->resourceManager->GetShader("UIShader"));

	float x = canvas->GetPosition().x + GetRect().x;
	float y = canvas->GetPosition().y + GetRect().y;

	glUseProgram(shader->shaderProgramID);
	float4x4 projectionMatrix = float4x4::FromTRS(float3(x, y, 0), Quat::FromEulerXYZ(0, 0, 0), float3(GetRect().w, GetRect().h, 1)).Transposed();
	glBindTexture(GL_TEXTURE_2D, id);
	shader->SetUniform1i("useColor", (GLint)true);
	shader->SetUniformMatrix4("projection", projectionMatrix.ptr());
	shader->SetUniformVec4f("inColor", (GLfloat*)&tempColor);

	glBindBuffer(GL_ARRAY_BUFFER, VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisable(GL_BLEND);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);*/
}

void C_UI_Checkbox::Draw3D()
{
}

UICheckboxState C_UI_Checkbox::GetState() const
{
	return state;
}

void C_UI_Checkbox::ResetInput()
{
	state = UICheckboxState::UNCHECKED;
}
