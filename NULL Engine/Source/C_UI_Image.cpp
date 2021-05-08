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

#include "C_UI_Image.h"

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
	glGenBuffers(1, &VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VAO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoordsBuffer), texCoordsBuffer, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (GLvoid*)0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void C_UI_Image::HandleInput(C_UI** selectedUi)
{
}

void C_UI_Image::Draw2D()
{
	uint32 id;
	C_2DAnimator* cAnimator = GetOwner()->GetComponent<C_2DAnimator>();
	C_Material* cMaterial = GetOwner()->GetComponent<C_Material>();

	if (cMaterial == nullptr) 
		return;

	else if (cAnimator && cAnimator->IsAnimationPlaying())
		id = cAnimator->GetIdFromAnimation();

	else
		id = cMaterial->GetTextureID();

	C_Canvas* canvas = GetOwner()->parent->GetComponent<C_Canvas>();
	if (canvas == nullptr) return;

	glEnable(GL_BLEND);

	if(!cMaterial->GetShader())
		cMaterial->SetShader(App->resourceManager->GetShader("UIShader"));

	glUseProgram(cMaterial->GetShader()->shaderProgramID);

	float x = canvas->GetPosition().x + GetRect().x;
	float y = canvas->GetPosition().y + GetRect().y;

	float4x4 projectionMatrix = float4x4::FromTRS(float3(x, y, 0), Quat::FromEulerXYZ(0, 0, 0), float3(GetRect().w, GetRect().h, 1)).Transposed();

	glBindTexture(GL_TEXTURE_2D, id);
	
	cMaterial->GetShader()->SetUniform1i("useColor", (GLint)false);
	cMaterial->GetShader()->SetUniformMatrix4("projection", projectionMatrix.ptr());

	glBindBuffer(GL_ARRAY_BUFFER, VAO);

	glDrawArrays(GL_TRIANGLES, 0, 6);


	glDisable(GL_BLEND);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	glUseProgram(0);
}

void C_UI_Image::Draw3D()
{
	uint32 id;
	C_2DAnimator* cAnimator = GetOwner()->GetComponent<C_2DAnimator>();
	C_Material* cMaterial = GetOwner()->GetComponent<C_Material>();


	if (cMaterial == nullptr && cAnimator == nullptr) 
		return;

	else if (cAnimator->IsAnimationPlaying())
		id = cAnimator->GetIdFromAnimation();

	else
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


bool C_UI_Image::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	root.SetNumber("X", GetRect().x);
	root.SetNumber("Y", GetRect().y);
	root.SetNumber("W", GetRect().w);
	root.SetNumber("H", GetRect().h);

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

	childOrder = root.GetNumber("childOrder");
	return true;
}