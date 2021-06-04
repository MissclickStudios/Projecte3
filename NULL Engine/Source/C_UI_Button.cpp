#include "Application.h"
#include "GameObject.h"

#include "M_Scene.h"
#include "M_Input.h"

#include "C_Material.h"
#include "C_Transform.h"

#include "M_ResourceManager.h"

#include "R_Shader.h"
#include "R_Texture.h"

#include "C_UI_Button.h"
#include "C_Canvas.h"

#include "Dependencies/glew/include/glew.h"
//#include "OpenGL.h"

#include "MemoryManager.h"

#include "JSONParser.h"

C_UI_Button::C_UI_Button(GameObject* owner, Rect2D rect) : C_UI(owner, ComponentType::UI_BUTTON, true, rect), state(UIButtonState::IDLE)
{
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
	// --- Delete Buffers
	glDeleteBuffers(1, (GLuint*)&VAO);
	glDeleteBuffers(1, (GLuint*)&VBO);
}

void C_UI_Button::LoadBuffers()
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
	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (cMaterial == nullptr || canvas == nullptr)
		return;
	
	//TODO: Inspector pick color !!!
	Color tempColor;
	switch (state)
	{
	case UIButtonState::IDLE:
		tempColor = idle; break;
	case UIButtonState::HOVERED:
		tempColor = hovered; break;
	case UIButtonState::PRESSEDIN:
		tempColor = pressed; break;
	case UIButtonState::PRESSED:
		tempColor = pressed; break;
	case UIButtonState::RELEASED:
		tempColor = pressed; break;
	default:
		tempColor = idle; break;
	}

	if (!cMaterial->GetShader())
		cMaterial->SetShader(App->resourceManager->GetShader("UIShader"));

	glEnable(GL_BLEND); //enabled in draw 2d render ui

	//Canvas position always returns 0,0 for 2d rendering
	float x = canvas->GetPosition().x + rect.x;
	float y = canvas->GetPosition().y + rect.y;

	glUseProgram(cMaterial->GetShader()->shaderProgramID);
	float4x4 projectionMatrix = float4x4::FromTRS(float3(x, y, 0), Quat::FromEulerXYZ(0, 0, 0), float3(GetRect().w, GetRect().h, 1)).Transposed();
	glBindTexture(GL_TEXTURE_2D, cMaterial->GetTextureID());
	cMaterial->GetShader()->SetUniform1i("useColor", (GLint)true);
	cMaterial->GetShader()->SetUniformMatrix4("projection", projectionMatrix.ptr());
	cMaterial->GetShader()->SetUniformVec4f("inColor", (GLfloat*)&tempColor);

	float newCoords[] = {
		0.0f, 1.0f, textCoord.proportionBeginX, textCoord.proportionFinalY,
		1.0f, 0.0f, textCoord.proportionFinalX, textCoord.proportionBeginY,
		0.0f, 0.0f, textCoord.proportionBeginX, textCoord.proportionBeginY,
												
		0.0f, 1.0f, textCoord.proportionBeginX, textCoord.proportionFinalY,
		1.0f, 1.0f, textCoord.proportionFinalX, textCoord.proportionFinalY,
		1.0f, 0.0f, textCoord.proportionFinalX, textCoord.proportionBeginY
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

void C_UI_Button::ResetInput()
{
	state = UIButtonState::IDLE;
}

Frame C_UI_Button::GetTexturePosition(int pixelPosX, int pixelPosY, int pixelWidth, int pixelHeight)
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

Frame C_UI_Button::GetTexturePosition(int pixelPosX, int pixelPosY, int pixelWidth, int pixelHeight, int textW, int textH)
{
	Frame frame;
	frame.proportionBeginX = (float)pixelPosX / textW;
	frame.proportionFinalX = ((float)pixelPosX + pixelWidth) / textW;

	frame.proportionBeginY = (float)pixelPosY / textH;
	frame.proportionFinalY = ((float)pixelPosY + pixelHeight) / textH;

	return frame;
}

bool C_UI_Button::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	root.SetNumber("X", rect.x);
	root.SetNumber("Y", rect.y);
	root.SetNumber("W", rect.w);
	root.SetNumber("H", rect.h);

	//color
	root.SetNumber("idler", idle.r); root.SetNumber("idleg", idle.g); root.SetNumber("idleb", idle.b); root.SetNumber("idlea", idle.a);
	root.SetNumber("hoveredr", hovered.r); root.SetNumber("hoveredg", hovered.g); root.SetNumber("hoveredb", hovered.b); root.SetNumber("hovereda", hovered.a);
	root.SetNumber("pressedr", pressed.r); root.SetNumber("pressedg", pressed.g); root.SetNumber("pressedb", pressed.b); root.SetNumber("presseda", pressed.a);
	
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
	//textCoords
	ParsonArray pixelCoords = root.SetArray("pixelCoords");
	for (int i = 0; i < 4; ++i)
		pixelCoords.SetNumber((double)pixelCoord[i]);

	ParsonNode node;
	node = root.SetNode("textureCoords");
	node.SetNumber("x", textCoord.proportionBeginX); node.SetNumber("y", textCoord.proportionBeginY);
	node.SetNumber("w", textCoord.proportionFinalX); node.SetNumber("h", textCoord.proportionFinalY);

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

	//color
	idle.r = root.GetNumber("idler"); idle.g = root.GetNumber("idleg"); idle.b = root.GetNumber("idleb"); idle.a =root.GetNumber("idlea");
	hovered.r = root.GetNumber("hoveredr"); hovered.g = root.GetNumber("hoveredg"); hovered.b = root.GetNumber("hoveredb"); hovered.a = root.GetNumber("hovereda");
	pressed.r = root.GetNumber("pressedr"); pressed.g = root.GetNumber("pressedg"); pressed.b = root.GetNumber("pressedb"); pressed.a = root.GetNumber("presseda");

	//textCoords
	ParsonArray pixelCoords = root.GetArray("pixelCoords");
	if (pixelCoords.ArrayIsValid())
		for (int i = 0; i < pixelCoords.size; ++i)
			pixelCoord[i] = (int)pixelCoords.GetNumber(i);

	ParsonNode size = root.GetNode("textureSize");
	if (size.NodeIsValid())
	{
		int spritesheetPixelWidth = size.GetInteger("textureWidth");
		int spritesheetPixelHeight = size.GetInteger("textureHeight");
		textCoord = GetTexturePosition(pixelCoord[0], pixelCoord[1], pixelCoord[2], pixelCoord[3], spritesheetPixelWidth, spritesheetPixelHeight);
	}
	else 
	{
		ParsonNode node;
		node = root.GetNode("textureCoords");
		if (node.NodeIsValid())
		{
			textCoord.proportionBeginX = node.GetNumber("x"); textCoord.proportionBeginY = node.GetNumber("y");
			textCoord.proportionFinalX = node.GetNumber("w"); textCoord.proportionFinalY = node.GetNumber("h");
		}
	}

	childOrder = root.GetInteger("childOrder");

	return true;
}

UIButtonState C_UI_Button::GetState() const
{
	return state;
}