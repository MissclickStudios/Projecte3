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

UI_Image::UI_Image(C_Canvas* canvas) : UIElement(canvas, UIElementType::IMAGE)
{

}

UI_Image::~UI_Image()
{

}

bool UI_Image::Update()
{
	bool ret = true;

	RenderImage();

	return ret;
}

bool UI_Image::CleanUp()
{
	bool ret = true;

	return ret;
}

void UI_Image::RenderImage()
{
	GameObject* go = GetCanvas()->GetOwner();
	if (go->GetComponent<C_Material>() == nullptr) return;
	if (go->GetComponent<C_Material>()->GetTextureID() == 0)
	{
		go->GetComponent<C_Material>()->SetUseDefaultTexture(true);
	}

	//Activates orthogonal but only for non master camera aka all component cameras
	if (App->camera->currentCamera != App->camera->masterCamera->GetComponent<C_Camera>())
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		// Not sure if it should be SceneTexture
		glOrtho(-App->editor->viewport->GetSceneTextureSize().x / 2, App->editor->viewport->GetSceneTextureSize().x / 2, -App->editor->viewport->GetSceneTextureSize().y / 2, App->editor->viewport->GetSceneTextureSize().y / 2, 100.0f, -100.0f);
		//glOrtho(-App->camera->GetCurrentCamera()->GetFrustum().NearPlaneWidth() / 2, App->camera->GetCurrentCamera()->GetFrustum().NearPlaneWidth() / 2, -App->camera->GetCurrentCamera()->GetFrustum().NearPlaneHeight() / 2, App->camera->GetCurrentCamera()->GetFrustum().NearPlaneHeight() / 2, 100.0f, -100.0f);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(App->camera->GetCurrentCamera()->GetOGLViewMatrix());
	}

	//glEnable(GL_BLEND);
	//
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glPushMatrix();
	glMultMatrixf((float*)&go->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	uint32 id = go->GetComponent<C_Material>()->GetTextureID();
	glBindTexture(GL_TEXTURE_2D, id); // Not sure
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(-GetCanvas()->GetSize().x / 2, -GetCanvas()->GetSize().y / 2);
	glTexCoord2f(1, 0); glVertex2f(GetCanvas()->GetSize().x / 2, -GetCanvas()->GetSize().y / 2);
	glTexCoord2f(1, 1); glVertex2f(GetCanvas()->GetSize().x / 2, GetCanvas()->GetSize().y / 2);
	glTexCoord2f(0, 1); glVertex2f(-GetCanvas()->GetSize().x / 2, GetCanvas()->GetSize().y / 2);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);

	glPopMatrix();

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	if (App->camera->currentCamera != App->camera->masterCamera->GetComponent<C_Camera>())
	{
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(App->camera->GetCurrentCamera()->GetOGLProjectionMatrix());
		glMatrixMode(GL_MODELVIEW);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}