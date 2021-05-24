#include "Application.h"
#include "C_UI_Slider.h"
#include "GameObject.h"
#include "C_Canvas.h"
#include "C_Material.h"
#include "R_Shader.h"
#include "M_ResourceManager.h"
#include "M_Input.h"
#include "Dependencies/glew/include/glew.h"
#include "JSONParser.h"

C_UI_Slider::C_UI_Slider(GameObject* owner, Rect2D rect) : C_UI(owner, ComponentType::UI_BUTTON, true, rect)
{
	LoadBuffers();
}

C_UI_Slider::~C_UI_Slider()
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

bool C_UI_Slider::Update()
{
	return true;
}

bool C_UI_Slider::CleanUp()
{
	return true;
}

bool C_UI_Slider::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	root.SetNumber("X", rect.x);
	root.SetNumber("Y", rect.y);
	root.SetNumber("W", rect.w);
	root.SetNumber("H", rect.h);

	ParsonArray pixelCoords = root.SetArray("pixelCoords");
	for (int i = 0; i < 16; ++i)
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

	root.SetInteger("childOrder", childOrder);
	return true;
}

bool C_UI_Slider::LoadState(ParsonNode& root)
{
	rect.x = root.GetNumber("X");
	rect.y = root.GetNumber("Y");
	rect.w = root.GetNumber("W");
	rect.h = root.GetNumber("H");

	ParsonArray pixelCoords = root.GetArray("pixelCoords");
	if (pixelCoords.ArrayIsValid())
		for (int i = 0; i < pixelCoords.size; ++i)
			pixelCoord[i] = (int)pixelCoords.GetNumber(i);

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

	childOrder = root.GetInteger("childOrder");
	return true;
}

void C_UI_Slider::LoadBuffers()
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

void C_UI_Slider::HandleInput(C_UI** selectedUi)
{
	if (!IsActive())
		return;
	if (!hovered) 
	{
		if (*selectedUi == nullptr || *selectedUi == this)
		{
			hovered = true;
			*selectedUi = this;
		}
	}
	else 
	{
		if (*selectedUi != this) 
		{
			hovered = false;
			return;
		}
		if (!trackedVariable)
			return;
		else if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KeyState::KEY_DOWN || App->input->GetGameControllerAxis(0) == AxisState::POSITIVE_AXIS_DOWN)
			*trackedVariable += (float)maxValue / (float)numRects;
		else if (App->input->GetKey(SDL_SCANCODE_LEFT) == KeyState::KEY_DOWN || App->input->GetGameControllerAxis(0) == AxisState::NEGATIVE_AXIS_DOWN)
			*trackedVariable -= (float)maxValue / (float)numRects;
	}
}

void C_UI_Slider::Draw2D()
{
	C_Material* cMaterial = GetOwner()->GetComponent<C_Material>();
	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (cMaterial == nullptr || canvas == nullptr)
		return;

	if (!cMaterial->GetShader())
		cMaterial->SetShader(App->resourceManager->GetShader("UIShader"));

	uint32 id = cMaterial->GetTextureID();

	glEnable(GL_BLEND); //enabled in draw 2d render ui

	//Canvas position always returns 0,0 for 2d rendering
	float x = canvas->GetPosition().x + rect.x;
	float y = canvas->GetPosition().y + rect.y;

	glUseProgram(cMaterial->GetShader()->shaderProgramID);
	glBindTexture(GL_TEXTURE_2D, id);

	int checkedRects = 0;
	if (*trackedVariable > maxValue)
		maxValue = *trackedVariable;

	checkedRects = ((*trackedVariable) * numRects) / maxValue;
	Frame currentFrame;
	if (hovered)
		currentFrame = hoverChecked;
	else
		currentFrame = unhoverUnchecked;

	for (int i = 0; i < checkedRects; ++i) 
	{
		float4x4 projectionMatrix = float4x4::FromTRS(float3(x, y, 0), Quat::FromEulerXYZ(0, 0, 0), float3(GetRect().w, GetRect().h, 1)).Transposed();
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
		x += offset + rect.w;
	}
	int uncheckedRects = numRects - checkedRects;
	if (hovered)
		currentFrame = hoverUnchecked;
	else
		currentFrame = unhoverUnchecked;

	for (int i = 0; i < uncheckedRects; ++i)
	{
		float4x4 projectionMatrix = float4x4::FromTRS(float3(x, y, 0), Quat::FromEulerXYZ(0, 0, 0), float3(GetRect().w, GetRect().h, 1)).Transposed();
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
		x += offset + rect.w;
	}

	glDisable(GL_BLEND);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void C_UI_Slider::Draw3D()
{
}

void C_UI_Slider::InputValue(float* value, float maxValue)
{
	if (value)
		return;

	if (maxValue != -1)
		this->maxValue = maxValue;

	trackedVariable = value;
	int checkedRects = ((*trackedVariable) * numRects) / maxValue;
	*trackedVariable = (float)numRects / this->maxValue * checkedRects;

}

void C_UI_Slider::ResetInput()
{
	hovered = false;
}

Frame C_UI_Slider::GetTexturePosition(int pixelPosX, int pixelPosY, int pixelWidth, int pixelHeight)
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