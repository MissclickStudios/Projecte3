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
#include "M_Audio.h"
#include "C_AudioSource.h"

C_UI_Slider::C_UI_Slider(GameObject* owner, Rect2D rect) : C_UI(owner, ComponentType::UI_SLIDER, true, rect)
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

	root.SetInteger("num rercts", numRects);
	root.SetNumber("max value", maxValue);
	root.SetNumber("offset", offset);
	
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

	numRects = root.GetInteger("num rercts");
	maxValue = root.GetNumber("max value");
	offset = root.GetNumber("offset");

	ParsonArray pixelCoords = root.GetArray("pixelCoords");
	if (pixelCoords.ArrayIsValid())
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
	}

	childOrder = root.GetInteger("childOrder");
	return true;
}

void C_UI_Slider::LoadBuffers()
{
	GLenum err = glGetError();
	while (err != GL_NO_ERROR)
	{
		LOG("OpenGl error: %d", err);
		err = glGetError();
	}
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
			hovered = false;
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
	checkedRects = (value * numRects) / maxValue;
	if (maxValue <= 0)
		checkedRects = 0;
	Frame currentFrame;
	if (hovered)
		currentFrame = hoverChecked;
	else
		currentFrame = unhoverChecked;

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
		GLenum err = glGetError();
		while (err != GL_NO_ERROR)
		{
			LOG("OpenGl error: %d", err);
			unsigned int a = sizeof(newCoords);
			int b = 500;
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &b);
			if (b != 96)
			{
				LOG("inputsize: %d existingSize: %d", a, b);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				// --- Delete Buffers
				glDeleteBuffers(1, (GLuint*)&VAO);
				glDeleteBuffers(1, (GLuint*)&VBO);
				// --- Rebuild the buffers
				glGenVertexArrays(1, &VAO);
				glGenBuffers(1, &VBO);

				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(newCoords), newCoords, GL_DYNAMIC_DRAW);

				glBindVertexArray(VAO);

				// position attribute
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);

				// texture coord attribute
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
				glEnableVertexAttribArray(1);

				glBindBuffer(GL_ARRAY_BUFFER, 0);
			}
			err = glGetError();
		}
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

float C_UI_Slider::InputValue(float value, float maxValue, int numSquares)
{
	if (value < 0)
		value = 0;
	if (maxValue >= 0)
		this->maxValue = maxValue;
	if (numSquares > 0)
		numRects = numSquares;
	if (value > this->maxValue)
		value = this->maxValue;
	
	int checkedRects = (value * numRects) / this->maxValue;
	this->value = checkedRects * (this->maxValue / ((float)numRects));
	return this->value;
}

float C_UI_Slider::IncrementOneSquare()
{
	App->audio->aSourceUi->SetEvent("ui_navigate");
	App->audio->aSourceUi->PlayFx(App->audio->aSourceUi->GetEventId());
	if(value < maxValue)
		value += (float)maxValue / (float)numRects;
	return value;
}

float C_UI_Slider::DecrementOneSquare()
{
	App->audio->aSourceUi->SetEvent("ui_navigate");
	App->audio->aSourceUi->PlayFx(App->audio->aSourceUi->GetEventId());
	if (value > 0) //min 0??
		value -= (float)maxValue / (float)numRects;
	return value;
}

float C_UI_Slider::GetSliderValue() const
{
	return value;
}

bool C_UI_Slider::Hovered() const
{
	return hovered;
}

void C_UI_Slider::Hoverable(bool setTo)
{
	interactuable = setTo;
	if (!interactuable)
		hovered = false;
}

void C_UI_Slider::ResetInput()
{
	hovered = false;
}