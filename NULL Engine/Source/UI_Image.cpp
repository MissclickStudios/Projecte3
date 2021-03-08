#include "Application.h"

#include "GameObject.h"

#include "M_Camera3D.h"
#include "M_Editor.h"

#include "C_Material.h"
#include "C_Canvas.h"
#include "C_Transform.h"
#include "C_Camera.h"

#include "E_Viewport.h"

#include "UI_Image.h"

#include "OpenGL.h"


#include "MemoryManager.h"


UI_Image::UI_Image(GameObject* owner, Rect rect) : UIElement(owner, UIElementType::IMAGE, rect)

{

}

UI_Image::~UI_Image()
{

}

bool UI_Image::Update()
{
	bool ret = true;

	if (IsActive() == false)
		return ret;

	if (GetCanvas() == nullptr)
		return ret;

	if (GetRect().w > GetCanvas()->GetRect().w)
		SetW(GetCanvas()->GetRect().w);

	if(GetRect().h > GetCanvas()->GetRect().h)
		SetH(GetCanvas()->GetRect().h);

	if (App->camera->currentCamera != App->camera->masterCamera->GetComponent<C_Camera>())
	{
		RenderImage2D();
	}
	else
	{
		RenderImage3D();
	}

	return ret;
}

bool UI_Image::CleanUp()
{
	bool ret = true;

	return ret;
}

void UI_Image::RenderImage2D()
{
	GameObject* go = GetOwner();

	// Revert UI changes
	if (GetCanvas()->GetOwner()->GetComponent<C_Material>() == nullptr) return;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-App->camera->GetCurrentCamera()->GetFrustum().NearPlaneWidth() / 2, App->camera->GetCurrentCamera()->GetFrustum().NearPlaneWidth() / 2, -App->camera->GetCurrentCamera()->GetFrustum().NearPlaneHeight() / 2, App->camera->GetCurrentCamera()->GetFrustum().NearPlaneHeight() / 2, 100.0f, -100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(App->camera->GetCurrentCamera()->GetOGLViewMatrix());

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);

	glPushMatrix();
	glMultMatrixf((GLfloat*)&GetCanvas()->GetOwner()->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	// Revert UI changes
	uint32 id = GetCanvas()->GetOwner()->GetComponent<C_Material>()->GetTextureID();
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(GetCanvas()->GetPosition().x + GetRect().x - GetRect().w / 2, GetCanvas()->GetPosition().y + GetRect().y - GetRect().h / 2);
	glTexCoord2f(1, 0); glVertex2f(GetCanvas()->GetPosition().x + GetRect().x + GetRect().w / 2, GetCanvas()->GetPosition().y + GetRect().y - GetRect().h / 2);
	glTexCoord2f(1, 1); glVertex2f(GetCanvas()->GetPosition().x + GetRect().x + GetRect().w / 2, GetCanvas()->GetPosition().y + GetRect().y + GetRect().h / 2);
	glTexCoord2f(0, 1); glVertex2f(GetCanvas()->GetPosition().x + GetRect().x - GetRect().w / 2, GetCanvas()->GetPosition().y + GetRect().y + GetRect().h / 2);
	glEnd();

	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(App->camera->GetCurrentCamera()->GetOGLProjectionMatrix());
	glMatrixMode(GL_MODELVIEW);
}

void UI_Image::RenderImage3D()
{
	GameObject* go = GetOwner();
	if (GetCanvas()->GetOwner()->GetComponent<C_Material>() == nullptr) return;
	
	glPushMatrix();
	glMultMatrixf((GLfloat*)&GetOwner()->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	uint32 id = GetCanvas()->GetOwner()->GetComponent<C_Material>()->GetTextureID();
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

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glPopMatrix();

}