#include "Application.h"

#include "GameObject.h"

#include "M_Camera3D.h"
#include "M_Scene.h"
#include "M_UISystem.h"

#include "M_ResourceManager.h"

#include "C_Material.h"
#include "C_Canvas.h"
#include "C_Transform.h"
#include "C_Camera.h"
#include "C_2DAnimator.h"

#include "R_Shader.h"
#include "R_Texture.h"
#include "Spritesheet.h"

#include "C_UI_Image.h"

#include "M_Input.h"

#include "Dependencies/glew/include/glew.h"
//#include "OpenGL.h"

#include "MemoryManager.h"

#include "JSONParser.h"

C_UI_Image::C_UI_Image(GameObject* owner, Rect2D rect) : C_UI(owner, ComponentType::UI_IMAGE, false, rect)
{
	LoadBuffers();
}

C_UI_Image::~C_UI_Image()
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

bool C_UI_Image::Update()
{
	return true;
}

bool C_UI_Image::CleanUp()
{
	return true;
}

void C_UI_Image::LoadBuffers()
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

void C_UI_Image::HandleInput(C_UI** selectedUi)
{
}

void C_UI_Image::Draw2D()
{
	uint32 id;
	C_2DAnimator* cAnimator = GetOwner()->GetComponent<C_2DAnimator>();
	C_Material* cMaterial = GetOwner()->GetComponent<C_Material>();
	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();

	if (cMaterial == nullptr || canvas == nullptr)
		return;

	else if (cAnimator && cAnimator->IsAnimationPlaying())
	{
		switch (cAnimator->GetAnimationNumber())
		{
		case 1:
			if (cAnimator->spritesheet != nullptr)
				id = cAnimator->spritesheet->spriteSheet->GetTextureID();
			break;
		case 2:
			if (cAnimator->spritesheet2 != nullptr)
				id = cAnimator->spritesheet2->spriteSheet->GetTextureID();
			break;
		case 3:
			if (cAnimator->spritesheet3 != nullptr)
				id = cAnimator->spritesheet3->spriteSheet->GetTextureID();
			break;
		}
	}
	else
		id = cMaterial->GetTextureID();

	glEnable(GL_BLEND);

	if (!cMaterial->GetShader())
		cMaterial->SetShader(App->resourceManager->GetShader("UIShader"));

	glUseProgram(cMaterial->GetShader()->shaderProgramID);

	float x = canvas->GetPosition().x + GetRect().x;
	float y = canvas->GetPosition().y + GetRect().y;

	float4x4 projectionMatrix = float4x4::FromTRS(float3(x, y, 0), Quat::FromEulerXYZ(0, 0, 0), float3(GetRect().w, GetRect().h, 1)).Transposed();

	//float4x4 identity = float4x4::identity;

	glBindTexture(GL_TEXTURE_2D, id);

	cMaterial->GetShader()->SetUniform1i("useColor", (GLint)true);
	cMaterial->GetShader()->SetUniformVec4f("inColor", (GLfloat*)&color);
	//cMaterial->GetShader()->SetUniformMatrix4("model", identity.Transposed().ptr());
	cMaterial->GetShader()->SetUniformMatrix4("projection", projectionMatrix.ptr());


	//Uncomment the code below to update the texture coords in real time
	if (cAnimator != nullptr && cAnimator->IsAnimationPlaying())
	{
		switch (cAnimator->GetAnimationNumber())
		{
		case 1:
		{
			float newCoords[] = {
	0.0f, 1.0f, cAnimator->spritesheet->currentFrame.proportionBeginX, cAnimator->spritesheet->currentFrame.proportionFinalY,
	1.0f, 0.0f, cAnimator->spritesheet->currentFrame.proportionFinalX, cAnimator->spritesheet->currentFrame.proportionBeginY,
	0.0f, 0.0f, cAnimator->spritesheet->currentFrame.proportionBeginX, cAnimator->spritesheet->currentFrame.proportionBeginY,

	0.0f, 1.0f, cAnimator->spritesheet->currentFrame.proportionBeginX, cAnimator->spritesheet->currentFrame.proportionFinalY,
	1.0f, 1.0f, cAnimator->spritesheet->currentFrame.proportionFinalX, cAnimator->spritesheet->currentFrame.proportionFinalY,
	1.0f, 0.0f, cAnimator->spritesheet->currentFrame.proportionFinalX,  cAnimator->spritesheet->currentFrame.proportionBeginY
			};

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newCoords), newCoords);
			break;
		}
		case 2:
		{
			float newCoordsSecond[] = {
	0.0f, 1.0f, cAnimator->spritesheet2->currentFrame.proportionBeginX, cAnimator->spritesheet2->currentFrame.proportionFinalY,
	1.0f, 0.0f, cAnimator->spritesheet2->currentFrame.proportionFinalX, cAnimator->spritesheet2->currentFrame.proportionBeginY,
	0.0f, 0.0f, cAnimator->spritesheet2->currentFrame.proportionBeginX, cAnimator->spritesheet2->currentFrame.proportionBeginY,

	0.0f, 1.0f, cAnimator->spritesheet2->currentFrame.proportionBeginX, cAnimator->spritesheet2->currentFrame.proportionFinalY,
	1.0f, 1.0f, cAnimator->spritesheet2->currentFrame.proportionFinalX, cAnimator->spritesheet2->currentFrame.proportionFinalY,
	1.0f, 0.0f, cAnimator->spritesheet2->currentFrame.proportionFinalX,  cAnimator->spritesheet2->currentFrame.proportionBeginY
			};

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newCoordsSecond), newCoordsSecond);
			break;
		}
		case 3:
		{
			float newCoordsThird[] = {
	0.0f, 1.0f, cAnimator->spritesheet3->currentFrame.proportionBeginX, cAnimator->spritesheet3->currentFrame.proportionFinalY,
	1.0f, 0.0f, cAnimator->spritesheet3->currentFrame.proportionFinalX, cAnimator->spritesheet3->currentFrame.proportionBeginY,
	0.0f, 0.0f, cAnimator->spritesheet3->currentFrame.proportionBeginX, cAnimator->spritesheet3->currentFrame.proportionBeginY,

	0.0f, 1.0f, cAnimator->spritesheet3->currentFrame.proportionBeginX, cAnimator->spritesheet3->currentFrame.proportionFinalY,
	1.0f, 1.0f, cAnimator->spritesheet3->currentFrame.proportionFinalX, cAnimator->spritesheet3->currentFrame.proportionFinalY,
	1.0f, 0.0f, cAnimator->spritesheet3->currentFrame.proportionFinalX,  cAnimator->spritesheet3->currentFrame.proportionBeginY
			};

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newCoordsThird), newCoordsThird);
			break;
		}
		}
	}
	else
	{
		float theCoords[] = {
		0.0f, 1.0f, textCoord.proportionBeginX, textCoord.proportionFinalY,
		1.0f, 0.0f, textCoord.proportionFinalX, textCoord.proportionBeginY,
		0.0f, 0.0f, textCoord.proportionBeginX, textCoord.proportionBeginY,

		0.0f, 1.0f, textCoord.proportionBeginX, textCoord.proportionFinalY,
		1.0f, 1.0f, textCoord.proportionFinalX, textCoord.proportionFinalY,
		1.0f, 0.0f, textCoord.proportionFinalX, textCoord.proportionBeginY
		};

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(theCoords), theCoords);
	}


	glBindVertexArray(VAO);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glDisable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}

void C_UI_Image::Draw3D()
{
	uint32 id;
	C_2DAnimator* cAnimator = GetOwner()->GetComponent<C_2DAnimator>();
	C_Material* cMaterial = GetOwner()->GetComponent<C_Material>();


	if (cMaterial == nullptr) 
		return;

	id = cMaterial->GetTextureID();

	glPushMatrix();
	glMultMatrixf((GLfloat*)&GetOwner()->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

	glEnable(GL_BLEND);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

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

void C_UI_Image::SetColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	color = Color(r/255.f, g/255.f, b/255.f, a/255.f);
}

void C_UI_Image::ResetColor()
{
	color = Color(1.0f, 1.0f, 1.0f, 1.0f);
}

Frame C_UI_Image::GetTexturePosition(int pixelPosX, int pixelPosY, int pixelWidth, int pixelHeight)
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

bool C_UI_Image::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	root.SetNumber("X", rect.x);
	root.SetNumber("Y", rect.y);
	root.SetNumber("W", rect.w);
	root.SetNumber("H", rect.h);

	/*//textCoords
	ParsonArray pixelCoords = root.SetArray("pixelCoords");
	for (int i = 0; i < 4; ++i)
		pixelCoords.SetNumber((double)pixelCoord[i]);*/

	ParsonNode colorNode;
	colorNode = root.SetNode("imgColor");
	colorNode.SetNumber("r", color.r); colorNode.SetNumber("g", color.g);
	colorNode.SetNumber("b", color.b); colorNode.SetNumber("a", color.a);


	ParsonNode textureNode;
	textureNode = root.SetNode("textureCoords");
	textureNode.SetNumber("x", textCoord.proportionBeginX); textureNode.SetNumber("y", textCoord.proportionBeginY);
	textureNode.SetNumber("w", textCoord.proportionFinalX); textureNode.SetNumber("h", textCoord.proportionFinalY);

	ParsonNode InsptextCoord;
	InsptextCoord = root.SetNode("inspectorTextureCoords");
	InsptextCoord.SetInteger("x", pixelCoord[0]); InsptextCoord.SetInteger("y", pixelCoord[1]);
	InsptextCoord.SetInteger("w", pixelCoord[2]); InsptextCoord.SetInteger("h", pixelCoord[3]);

	 root.SetNumber("childOrder", childOrder);

	return true;
}

bool C_UI_Image::LoadState(ParsonNode& root)
{
	/*ParsonNode image = root.GetNode("Image");
	rect.x = image.GetNumber("X");
	rect.y = image.GetNumber("Y");
	rect.w = image.GetNumber("W");
	rect.h = image.GetNumber("H");*/

	rect.x = root.GetNumber("X");
	rect.y = root.GetNumber("Y");
	rect.w = root.GetNumber("W");
	rect.h = root.GetNumber("H");

	//textCoords
	/*ParsonArray pixelCoords = root.GetArray("pixelCoords");
	if (pixelCoords.ArrayIsValid())
		for (int i = 0; i < pixelCoords.size; ++i)
			pixelCoord[i] = (int)pixelCoords.GetNumber(i);*/
	ParsonNode InsptextCoord;
	InsptextCoord = root.GetNode("inspectorTextureCoords");
	if (InsptextCoord.NodeIsValid()) 
	{
		pixelCoord[0] = InsptextCoord.GetInteger("x"); pixelCoord[1] = InsptextCoord.GetInteger("y");
		pixelCoord[2] = InsptextCoord.GetInteger("w"); pixelCoord[3] = InsptextCoord.GetInteger("h");
	}

	ParsonNode colorNode;
	colorNode = root.GetNode("imgColor");
	if (colorNode.NodeIsValid())
	{
		color.r = colorNode.GetNumber("r"); color.g = colorNode.GetNumber("g");
		color.b = colorNode.GetNumber("b"); color.a = colorNode.GetNumber("a");
	}
	
	ParsonNode node;
	node = root.GetNode("textureCoords");
	if (node.NodeIsValid())
	{
		textCoord.proportionBeginX = node.GetNumber("x"); textCoord.proportionBeginY = node.GetNumber("y");
		textCoord.proportionFinalX = node.GetNumber("w"); textCoord.proportionFinalY = node.GetNumber("h");
	}

	childOrder = root.GetNumber("childOrder");
	return true;
}