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
	// --- Delete Buffers
	glDeleteBuffers(1, (GLuint*)&VAO);
	glDeleteBuffers(1, (GLuint*)&VBO);
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

	C_Material* cMaterial = GetOwner()->GetComponent<C_Material>();
	if (cMaterial)
	{
		uint32 id = cMaterial->GetTextureID();
		unsigned int spritesheetPixelWidth, spritesheetPixelHeight = 0; cMaterial->GetTextureSize(spritesheetPixelWidth, spritesheetPixelHeight);
		if (spritesheetPixelWidth && spritesheetPixelHeight)
		{
			ParsonNode size = root.SetNode("textureSize");
			size.SetInteger("textureWidth", spritesheetPixelWidth);
			size.SetInteger("textureHeight", spritesheetPixelHeight);
		}
	}
	ParsonArray pixelCoords = root.SetArray("pixelCoords");
	for (int i = 0; i < 24; ++i)
		pixelCoords.SetNumber((double)pixelCoord[i]);

	ParsonNode node;
	node = root.SetNode("unhover uncheck");
	node.SetNumber("x", unhoverUnchecked.proportionBeginX); node.SetNumber("y", unhoverUnchecked.proportionBeginY);
	node.SetNumber("w", unhoverUnchecked.proportionFinalX); node.SetNumber("h", unhoverUnchecked.proportionFinalY);
	node = root.SetNode("hover uncheck");
	node.SetNumber("x", hoverUnchecked.proportionBeginX); node.SetNumber("y", hoverUnchecked.proportionBeginY);
	node.SetNumber("w", hoverUnchecked.proportionFinalX); node.SetNumber("h", hoverUnchecked.proportionFinalY);
	node = root.SetNode("unhover check");
	node.SetNumber("x", unhoverChecked.proportionBeginX); node.SetNumber("y", unhoverChecked.proportionBeginY);
	node.SetNumber("w", unhoverChecked.proportionFinalX); node.SetNumber("h", unhoverChecked.proportionFinalY);
	node = root.SetNode("hover check");
	node.SetNumber("x", hoverChecked.proportionBeginX); node.SetNumber("y", hoverChecked.proportionBeginY);
	node.SetNumber("w", hoverChecked.proportionFinalX); node.SetNumber("h", hoverChecked.proportionFinalY);
	node = root.SetNode("pressed check");
	node.SetNumber("x", pressedChecked.proportionBeginX); node.SetNumber("y", pressedChecked.proportionBeginY);
	node.SetNumber("w", pressedChecked.proportionFinalX); node.SetNumber("h", pressedChecked.proportionFinalY);
	node = root.SetNode("pressed uncheck");
	node.SetNumber("x", pressedUnchecked.proportionBeginX); node.SetNumber("y", pressedUnchecked.proportionBeginY);
	node.SetNumber("w", pressedUnchecked.proportionFinalX); node.SetNumber("h", pressedUnchecked.proportionFinalY);

	root.SetInteger("childOrder", childOrder);
	return true;
}

bool C_UI_Checkbox::LoadState(ParsonNode& root)
{
	rect.x = root.GetNumber("X");
	rect.y = root.GetNumber("Y");
	rect.w = root.GetNumber("W");
	rect.h = root.GetNumber("H");

	ParsonArray pixelCoords = root.GetArray("pixelCoords");
	if(pixelCoords.ArrayIsValid())
		for (int i = 0; i < pixelCoords.size; ++i)
			pixelCoord[i] = (int)pixelCoords.GetNumber(i);

	ParsonNode size = root.GetNode("textureSize");
	if (size.NodeIsValid())
	{
		int spritesheetPixelWidth = size.GetInteger("textureWidth");
		int spritesheetPixelHeight = size.GetInteger("textureHeight");
		unhoverUnchecked = GetTexturePosition(pixelCoord[0], pixelCoord[1], pixelCoord[2], pixelCoord[3], spritesheetPixelWidth, spritesheetPixelHeight);
		hoverUnchecked = GetTexturePosition(pixelCoord[4], pixelCoord[5], pixelCoord[6], pixelCoord[7], spritesheetPixelWidth, spritesheetPixelHeight);
		unhoverChecked = GetTexturePosition(pixelCoord[8], pixelCoord[9], pixelCoord[10], pixelCoord[11], spritesheetPixelWidth, spritesheetPixelHeight);
		hoverChecked = GetTexturePosition(pixelCoord[12], pixelCoord[13], pixelCoord[14], pixelCoord[15], spritesheetPixelWidth, spritesheetPixelHeight);
		pressedChecked = GetTexturePosition(pixelCoord[16], pixelCoord[17], pixelCoord[18], pixelCoord[19], spritesheetPixelWidth, spritesheetPixelHeight);
		pressedUnchecked = GetTexturePosition(pixelCoord[20], pixelCoord[21], pixelCoord[22], pixelCoord[23], spritesheetPixelWidth, spritesheetPixelHeight);
	}
	else
	{
		ParsonNode node;
		node = root.GetNode("unhover uncheck");
		if (node.NodeIsValid())
		{
			unhoverUnchecked.proportionBeginX = node.GetNumber("x"); unhoverUnchecked.proportionBeginY = node.GetNumber("y");
			unhoverUnchecked.proportionFinalX = node.GetNumber("w"); unhoverUnchecked.proportionFinalY = node.GetNumber("h");
		}
		node = root.GetNode("hover uncheck");
		if (node.NodeIsValid())
		{
			hoverUnchecked.proportionBeginX = node.GetNumber("x"); hoverUnchecked.proportionBeginY = node.GetNumber("y");
			hoverUnchecked.proportionFinalX = node.GetNumber("w"); hoverUnchecked.proportionFinalY = node.GetNumber("h");
		}
		node = root.GetNode("unhover check");
		if (node.NodeIsValid())
		{
			unhoverChecked.proportionBeginX = node.GetNumber("x"); unhoverChecked.proportionBeginY = node.GetNumber("y");
			unhoverChecked.proportionFinalX = node.GetNumber("w"); unhoverChecked.proportionFinalY = node.GetNumber("h");
		}
		node = root.GetNode("hover check");
		if (node.NodeIsValid())
		{
			hoverChecked.proportionBeginX = node.GetNumber("x"); hoverChecked.proportionBeginY = node.GetNumber("y");
			hoverChecked.proportionFinalX = node.GetNumber("w"); hoverChecked.proportionFinalY = node.GetNumber("h");
		}
		node = root.GetNode("pressed check");
		if (node.NodeIsValid())
		{
			pressedChecked.proportionBeginX = node.GetNumber("x"); pressedChecked.proportionBeginY = node.GetNumber("y");
			pressedChecked.proportionFinalX = node.GetNumber("w"); pressedChecked.proportionFinalY = node.GetNumber("h");
		}
		node = root.GetNode("pressed uncheck");
		if (node.NodeIsValid())
		{
			pressedUnchecked.proportionBeginX = node.GetNumber("x"); pressedUnchecked.proportionBeginY = node.GetNumber("y");
			pressedUnchecked.proportionFinalX = node.GetNumber("w"); pressedUnchecked.proportionFinalY = node.GetNumber("h");
		}
	}
	
	childOrder = root.GetInteger("childOrder");
	return true;
}

void C_UI_Checkbox::SetChecked()
{
	state = UICheckboxState::CHECKED;
}

void C_UI_Checkbox::SetUnchecked()
{
	state = UICheckboxState::UNCHECKED;
}

void C_UI_Checkbox::LoadBuffers()
{
	const float texCoordsBuffer[] = {
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoordsBuffer), texCoordsBuffer, GL_DYNAMIC_DRAW);

	glBindVertexArray(VAO);

	// position attribute
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
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
			state = UICheckboxState::HOVERED_UNCHECKED;
			*selectedUi = this;
		}
		break;

	case UICheckboxState::CHECKED:
		if (*selectedUi == nullptr || *selectedUi == this)
		{
			state = UICheckboxState::HOVERED_CHECKED;
			*selectedUi = this;
		}
		break;

	case UICheckboxState::HOVERED_UNCHECKED:
		if (*selectedUi != this)
		{
			state = UICheckboxState::UNCHECKED;
			break;
		}
		if (App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_DOWN) // TODO: make inputs mappable
			state = UICheckboxState::PRESSED_UNCHECKED_IN;
		break;

	case UICheckboxState::HOVERED_CHECKED:
		if (*selectedUi != this)
		{
			state = UICheckboxState::CHECKED;
			break;
		}
		if (App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_DOWN || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_DOWN) // TODO: make inputs mappable
			state = UICheckboxState::PRESSED_CHECKED_IN;
		break;

	case UICheckboxState::PRESSED_CHECKED_IN:
		state = UICheckboxState::PRESSED_CHECKED;
		break;

	case UICheckboxState::PRESSED_CHECKED:
		if (*selectedUi != this)
		{
			state = UICheckboxState::CHECKED;
			break;
		}
		if (App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_UP || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_UP)
		{
			state = UICheckboxState::PRESSED_CHECKED_OUT;
		}
		break;

	case UICheckboxState::PRESSED_CHECKED_OUT:
		state = UICheckboxState::HOVERED_UNCHECKED;
		break;

	case UICheckboxState::PRESSED_UNCHECKED_IN:
		state = UICheckboxState::PRESSED_UNCHECKED;
		break;

	case UICheckboxState::PRESSED_UNCHECKED:
		if (*selectedUi != this)
		{
			state = UICheckboxState::UNCHECKED;
			break;
		}
		if (App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_UP || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_UP)
		{
			state = UICheckboxState::PRESSED_UNCHECKED_OUT;
		}
		break;

	case UICheckboxState::PRESSED_UNCHECKED_OUT:
		state = UICheckboxState::HOVERED_CHECKED;
		break;

	default:
		state = UICheckboxState::UNCHECKED;
		break;
	}
}

void C_UI_Checkbox::Draw2D()
{
	C_Material* cMaterial = GetOwner()->GetComponent<C_Material>();
	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (cMaterial == nullptr || canvas == nullptr)
		return;

	Frame currentFrame;
	switch (state)
	{
	case UICheckboxState::NONE: currentFrame = unhoverUnchecked;  break;
	case UICheckboxState::UNCHECKED: currentFrame = unhoverUnchecked; break;
	case UICheckboxState::CHECKED: currentFrame = unhoverChecked; break;
	case UICheckboxState::HOVERED_UNCHECKED: currentFrame = hoverUnchecked; break;
	case UICheckboxState::HOVERED_CHECKED: currentFrame = hoverChecked; break;
	case UICheckboxState::PRESSED_CHECKED_IN: currentFrame = pressedChecked; break;
	case UICheckboxState::PRESSED_CHECKED: currentFrame = pressedChecked; break;
	case UICheckboxState::PRESSED_CHECKED_OUT: currentFrame = pressedChecked; break;
	case UICheckboxState::PRESSED_UNCHECKED_IN: currentFrame = pressedUnchecked; break;
	case UICheckboxState::PRESSED_UNCHECKED: currentFrame = pressedUnchecked; break;
	case UICheckboxState::PRESSED_UNCHECKED_OUT: currentFrame = pressedUnchecked; break;
	default: currentFrame = unhoverUnchecked; break;
	}

	if (!cMaterial->GetShader())
		cMaterial->SetShader(App->resourceManager->GetShader("UIShader"));

	uint32 id = cMaterial->GetTextureID();

	glEnable(GL_BLEND); //enabled in draw 2d render ui

	//Canvas position always returns 0,0 for 2d rendering
	float x = canvas->GetPosition().x + rect.x;
	float y = canvas->GetPosition().y + rect.y;

	glUseProgram(cMaterial->GetShader()->shaderProgramID);
	float4x4 projectionMatrix = float4x4::FromTRS(float3(x, y, 0), Quat::FromEulerXYZ(0, 0, 0), float3(GetRect().w, GetRect().h, 1)).Transposed();
	glBindTexture(GL_TEXTURE_2D, id);
	cMaterial->GetShader()->SetUniform1i("useColor", (GLint)false);
	cMaterial->GetShader()->SetUniformMatrix4("projection", projectionMatrix.ptr());
	//cMaterial->GetShader()->SetUniformVec4f("inColor", (GLfloat*)&tempColor);

	float newCoords[] = {
		0.0f, 1.0f, currentFrame.proportionBeginX, currentFrame.proportionFinalY,
		1.0f, 0.0f, currentFrame.proportionFinalX, currentFrame.proportionBeginY,
		0.0f, 0.0f, currentFrame.proportionBeginX, currentFrame.proportionBeginY,

		0.0f, 1.0f, currentFrame.proportionBeginX, currentFrame.proportionFinalY,
		1.0f, 1.0f, currentFrame.proportionFinalX, currentFrame.proportionFinalY,
		1.0f, 0.0f, currentFrame.proportionFinalX, currentFrame.proportionBeginY
	};

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newCoords), newCoords);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisable(GL_BLEND);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

const char* C_UI_Checkbox::NameFromState(UICheckboxState state)
{
	switch (state)
	{
	case UICheckboxState::NONE: return "UNCHECKED";
	case UICheckboxState::UNCHECKED: return "UNCHECKED";
	case UICheckboxState::CHECKED: return "CHECKED";
	case UICheckboxState::HOVERED_UNCHECKED: return "HOVERED_UNCHECKED";
	case UICheckboxState::HOVERED_CHECKED: return "HOVERED_CHECKED";
	case UICheckboxState::PRESSED_CHECKED_IN: return "PRESSED_CHECKED";
	case UICheckboxState::PRESSED_CHECKED: return "PRESSED_CHECKED";
	case UICheckboxState::PRESSED_CHECKED_OUT: return "PRESSED_CHECKED";
	case UICheckboxState::PRESSED_UNCHECKED_IN: return "PRESSED_UNCHECKED";
	case UICheckboxState::PRESSED_UNCHECKED: return "PRESSED_UNCHECKED";
	case UICheckboxState::PRESSED_UNCHECKED_OUT: return "PRESSED_UNCHECKED";
	default: return "UNCHECKED";
	}
}

void C_UI_Checkbox::Draw3D()
{
}

void C_UI_Checkbox::Check()
{
	state = UICheckboxState::CHECKED;
}

void C_UI_Checkbox::UnCheck()
{
	state = UICheckboxState::UNCHECKED;
}

UICheckboxState C_UI_Checkbox::GetState() const
{
	return state;
}

void C_UI_Checkbox::ResetInput()
{
	state = UICheckboxState::UNCHECKED;
}
