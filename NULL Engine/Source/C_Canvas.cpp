#include "Application.h"

#include "M_Renderer3D.h"
#include "M_Camera3D.h"
#include "M_Editor.h"
#include "M_Window.h"

#include "GameObject.h"

#include "C_Camera.h"
#include "C_Transform.h"
#include "C_Canvas.h"

#include "E_Viewport.h"

#include "OpenGL.h"

#include "JSONParser.h"


C_Canvas::C_Canvas(GameObject* owner) : Component(owner, ComponentType::CANVAS)
{
	pivot = GetPosition();
	isInvisible = false;
}

C_Canvas::~C_Canvas()
{

}

bool C_Canvas::Update()
{
	bool ret = true;

	//This will be world space only
	if (IsActive())
	{
		GameObject* owner = GetOwner();
		SetPosition({ owner->transform->GetWorldPosition().x, owner->transform->GetWorldPosition().y });
		z = owner->transform->GetWorldPosition().z;
	}

	return ret;
}

void C_Canvas::Draw2D()
{
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//// Not sure if it should be SceneTexture
	////glOrtho(-App->editor->viewport->GetSceneTextureSize().x / 2, App->editor->viewport->GetSceneTextureSize().x / 2, -App->editor->viewport->GetSceneTextureSize().y / 2, App->editor->viewport->GetSceneTextureSize().y / 2, 100.0f, -100.0f);
	//glOrtho(-App->camera->GetCurrentCamera()->GetFrustum().NearPlaneWidth() / 2, App->camera->GetCurrentCamera()->GetFrustum().NearPlaneWidth() / 2, -App->camera->GetCurrentCamera()->GetFrustum().NearPlaneHeight() / 2, App->camera->GetCurrentCamera()->GetFrustum().NearPlaneHeight() / 2, 100.0f, -100.0f);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadMatrixf(App->camera->GetCurrentCamera()->GetOGLViewMatrix());

	glLineWidth(2.0f);

	glBegin(GL_LINES);

	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);										// X Axis.
	glVertex2f(rect.x - rect.w / 2, rect.y + rect.h / 2);			glVertex2f(rect.x + rect.w / 2, rect.y + rect.h / 2);
	glVertex2f(rect.x + rect.w / 2, rect.y + rect.h / 2);			glVertex2f(rect.x + rect.w / 2, rect.y - rect.h / 2);
	glVertex2f(rect.x + rect.w / 2, rect.y - rect.h / 2);			glVertex2f(rect.x - rect.w / 2, rect.y - rect.h / 2);
	glVertex2f(rect.x - rect.w / 2, rect.y - rect.h / 2);			glVertex2f(rect.x - rect.w / 2, rect.y + rect.h / 2);

	glEnd();

	//Pivot
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 50; i++)
	{
		float angle = 2.0f * 3.1415926f * float(i) / float(50);				//get the current angle

		float sizeAv = (rect.w + rect.h) / 80;
		float x = sizeAv * cosf(angle);										//calculate the x component
		float y = sizeAv * sinf(angle);										//calculate the y component

		glVertex2f(rect.x + pivot.x + x, rect.y + pivot.y + y);		//output vertex

	}
	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(1.0f);

	/*glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(App->camera->GetCurrentCamera()->GetOGLProjectionMatrix());
	glMatrixMode(GL_MODELVIEW);
	*/
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

void C_Canvas::Draw3D()
{
	glPushMatrix();
	glMultMatrixf((GLfloat*)&GetOwner()->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

	glLineWidth(2.0f);

	glBegin(GL_LINES);

	glColor4f(1.0f, 0.0f, 0.0f, 1.0f);										// X Axis.
	glVertex2f(- rect.w / 2, + rect.h / 2);			glVertex2f(+ rect.w / 2, + rect.h / 2);
	glVertex2f(+ rect.w / 2, + rect.h / 2);			glVertex2f(+ rect.w / 2, - rect.h / 2);
	glVertex2f(+ rect.w / 2, - rect.h / 2);			glVertex2f(- rect.w / 2, - rect.h / 2);
	glVertex2f(- rect.w / 2, - rect.h / 2);			glVertex2f(- rect.w / 2, + rect.h / 2);

	glEnd();

	//Pivot
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 50; i++)
	{
		float angle = 2.0f * 3.1415926f * float(i) / float(50);				

		float sizeAv = (rect.w + rect.h) / 80;
		float x = sizeAv * cosf(angle);										
		float y = sizeAv * sinf(angle);										

		glVertex3f(pivot.x + x, pivot.y + y, 0);							

	}
	glEnd();

	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glLineWidth(1.0f);

	glPopMatrix();

}

bool C_Canvas::CleanUp()
{
	bool ret = true;

	return ret;
}

bool C_Canvas::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (uint)GetType());

	ParsonNode canvas = root.SetNode("Canvas");

	canvas.SetNumber("X", GetRect().x);
	canvas.SetNumber("Y", GetRect().y);
	canvas.SetNumber("W", GetRect().w);
	canvas.SetNumber("H", GetRect().h);
	canvas.SetNumber("Z", GetZ());

	return ret;
}

bool C_Canvas::LoadState(ParsonNode& root)
{
	bool ret = true;

	ParsonNode canvas = root.GetNode("Canvas");

	Rect r;

	r.x = canvas.GetNumber("X");
	r.y = canvas.GetNumber("Y");
	r.w = canvas.GetNumber("W");
	r.h = canvas.GetNumber("H");

	SetRect(r);
	SetZ(canvas.GetNumber("Z"));

	return ret;
}

float2 C_Canvas::GetPosition() const
{
	return { rect.x, rect.y };
}

float2 C_Canvas::GetSize() const
{
	return { rect.w, rect.h };
}

Rect C_Canvas::GetRect() const
{
	return rect;
}

float C_Canvas::GetZ() const
{
	return z;
}


void C_Canvas::SetPosition(const float2& position)
{
	this->rect.x = position.x;
	this->rect.y = position.y;
}

void C_Canvas::SetSize(const float2& size)
{
	this->rect.w = size.x;
	this->rect.h = size.y;
}

void C_Canvas::SetRect(const Rect& rect)
{
	this->rect.x = rect.x;
	this->rect.y = rect.y;
	this->rect.w = rect.w;
	this->rect.h = rect.h;
}

void C_Canvas::SetIsInvisible(const bool setTo)
{
	isInvisible = setTo;
}

void C_Canvas::SetZ(const float& z)
{
	this->z = z;
}

bool C_Canvas::IsInvisible() const
{
	return isInvisible;
}