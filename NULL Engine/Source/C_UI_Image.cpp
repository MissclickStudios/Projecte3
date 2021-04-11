#include "Application.h"

#include "GameObject.h"

#include "M_Camera3D.h"
#include "M_Editor.h"
#include "M_Scene.h"
#include "M_ResourceManager.h"

#include "C_Material.h"
#include "C_Canvas.h"
#include "C_Transform.h"
#include "C_Camera.h"
#include "C_2DAnimator.h"

#include "R_Shader.h"

#include "E_Viewport.h"

#include "C_UI_Image.h"

#include "OpenGL.h"

#include "MemoryManager.h"

#include "JSONParser.h"

C_UI_Image::C_UI_Image(GameObject* owner, Rect2D rect) : Component(owner, ComponentType::UI_IMAGE)
{
	rShader = App->resourceManager->GetShader("UIShader");
	LoadBuffers();
	
	SetW(2);
	SetH(2);

	SetX(0);
	SetY(0);
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

	//if (GetRect().w > canvas->GetRect().w)
	//	SetW(canvas->GetRect().w);

	//if(GetRect().h > canvas->GetRect().h)
	//	SetH(canvas->GetRect().h);

	return ret;
}

bool C_UI_Image::CleanUp()
{
	bool ret = true;

	return ret;
}

void C_UI_Image::LoadBuffers()
{
	glGenBuffers(1, &VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VAO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoordsBuffer), texCoordsBuffer, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void C_UI_Image::Draw2D()
{
	if (GetOwner()->GetComponent<C_Material>() == nullptr && GetOwner()->GetComponent<C_2DAnimator>() == nullptr) return;

	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (canvas == nullptr) return;

	glUseProgram(rShader->shaderProgramID);

	float x = canvas->GetPosition().x + GetRect().x;
	float y = canvas->GetPosition().y + GetRect().y;

	float4x4 projectionMatrix = float4x4::FromTRS(float3(x, y, 0), Quat::FromEulerXYZ(0, 0, 0), float3(GetRect().w, GetRect().h, 1)).Transposed();

	uint32 textureID;

	if (GetOwner()->GetComponent<C_2DAnimator>() != nullptr)
		textureID = GetOwner()->GetComponent<C_2DAnimator>()->GetIdFromAnimation();
	else
		textureID = GetOwner()->GetComponent<C_Material>()->GetTextureID();

	glBindTexture(GL_TEXTURE_2D, textureID);

	rShader->SetUniformMatrix4("projection", projectionMatrix.ptr());

	glBindBuffer(GL_ARRAY_BUFFER, VAO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (GLvoid*)0);

	glDrawArrays(GL_TRIANGLES, 0, 6);


	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}

void C_UI_Image::Draw3D()
{
	if (GetOwner()->GetComponent<C_Material>() == nullptr && GetOwner()->GetComponent<C_2DAnimator>() == nullptr) return;
	
	glPushMatrix();
	glMultMatrixf((GLfloat*)&GetOwner()->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

	glEnable(GL_BLEND);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	uint32 id;

	if (GetOwner()->GetComponent<C_2DAnimator>() != nullptr)
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