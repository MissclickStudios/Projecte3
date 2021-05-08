#include "Application.h"

#include "Profiler.h"
#include "GameObject.h"

#include "M_Camera3D.h"
#include "M_Scene.h"
#include "M_Input.h"

#include "C_Material.h"
#include "C_Transform.h"
#include "C_Camera.h"

#include "M_ResourceManager.h"

#include "R_Shader.h"
#include "R_Texture.h"

#include "C_UI_Button.h"
#include "C_Canvas.h"

#include "Dependencies/glew/include/glew.h"
//#include "OpenGL.h"

#include "MemoryManager.h"

#include "JSONParser.h"

C_UI_Button::C_UI_Button(GameObject* owner, Rect2D rect) : C_UI(owner, ComponentType::UI_BUTTON, true, rect)
{
	state = UIButtonState::IDLE;

	//TODO: posarlo en el vector del component canvas !!!!!!!
	LoadBuffers();
}

C_UI_Button::~C_UI_Button()
{
	GameObject* parent = GetOwner()->parent;
	if (parent)
	{
		C_Canvas* canvas = parent->GetComponent<C_Canvas>();
		if (canvas)
			canvas->RemoveUiElement(this);
	}
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
	//TODO: check de reparenting !!! (mirar si he canviat de owner) !!!!!!!
	return true;
}

bool C_UI_Button::CleanUp()
{
	return true;
}

void C_UI_Button::HandleInput(C_UI** selectedUi)
{
	if (!IsActive())
		return;

	switch (state)
	{
	case UIButtonState::IDLE:
		if (*selectedUi == nullptr || *selectedUi == this)
		{
			state = UIButtonState::HOVERED;
			*selectedUi = this;
		}
		break;
	case UIButtonState::HOVERED:
		if (*selectedUi != this)
		{
			state = UIButtonState::IDLE;
			break;
		}
		if (App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_DOWN) // TODO: make inputs mappable
		{
			state = UIButtonState::PRESSEDIN;
		}
		break;
	case UIButtonState::PRESSEDIN:
		state = UIButtonState::PRESSED;
		break;
	case UIButtonState::PRESSED:
		if (*selectedUi != this)
		{
			state = UIButtonState::IDLE;
			break;
		}
		if (App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_UP || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_UP)
		{
			state = UIButtonState::RELEASED;
		}
		break;
	case UIButtonState::RELEASED:
		state = UIButtonState::HOVERED;
		break;
	default:
		state = UIButtonState::IDLE; 
		break;
	}
}

void C_UI_Button::Draw2D()
{
	C_Material* cMaterial = GetOwner()->GetComponent<C_Material>();
	if (cMaterial == nullptr) 
		return;
	
	//TODO: Inspector pick color !!!
	Color tempColor;
	switch (state)
	{
	case UIButtonState::IDLE:
		tempColor = Color(1.0f, 1.0f, 0.0f, 1.0f); break;
	case UIButtonState::HOVERED:
		tempColor = Color(1.0f, 1.0f, 1.0f, 1.0f); break;
	case UIButtonState::PRESSEDIN:
		tempColor = Color(1.0f, 0.4f, 0.0f, 1.0f); break;
	case UIButtonState::PRESSED:
		tempColor = Color(1.0f, 0.4f, 0.0f, 1.0f); break;
	case UIButtonState::RELEASED:
		tempColor = Color(1.0f, 0.4f, 0.0f, 1.0f); break;
	default:
		tempColor = Color(1.0f, 1.0f, 0.0f, 1.0f); break;
	}

	if (!cMaterial->GetShader())
		return;
	R_Shader* shader = cMaterial->GetShader();
	uint32 id = cMaterial->GetTextureID();

	//TODO: Parent of parent draw (simplement buscar un C_UI i si no return)
	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (canvas == nullptr)
	{
		return;
	}
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

bool C_UI_Button::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	root.SetNumber("X", GetRect().x);
	root.SetNumber("Y", GetRect().y);
	root.SetNumber("W", GetRect().w);
	root.SetNumber("H", GetRect().h);

	root.SetInteger("childOrder", childOrder);
	return true;
}

bool C_UI_Button::LoadState(ParsonNode& root)
{
	/*ParsonNode button = root.GetNode("Button");
	rect.x = button.GetNumber("X");
	rect.y = button.GetNumber("Y");
	rect.w = button.GetNumber("W");
	rect.h = button.GetNumber("H");*/

	rect.x = root.GetNumber("X");
	rect.y = root.GetNumber("Y");
	rect.w = root.GetNumber("W");
	rect.h = root.GetNumber("H");

	childOrder = root.GetInteger("childOrder");

	return true;
}

UIButtonState C_UI_Button::GetState() const
{
	return state;
}