#include "Application.h"

#include "Profiler.h"

#include "M_Renderer3D.h"
#include "M_Camera3D.h"
#include "M_Window.h"
#include "M_UISystem.h"
//#include <list> included by module uiSystem
#include "M_Scene.h"
#include "M_Input.h"
#include "M_Audio.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Canvas.h"
#include "C_UI.h"
#include "C_UI_Button.h"
#include "C_AudioSource.h"

#include "Dependencies/glew/include/glew.h"
//#include "OpenGL.h"

#include "MemoryManager.h"

#include "JSONParser.h"


C_Canvas::C_Canvas(GameObject* owner) : Component(owner, ComponentType::CANVAS)
{
	App->uiSystem->allCanvas.push_back(this);
}

C_Canvas::~C_Canvas()
{
	//TODO: delete from active canvas list???? (if not it crashes if i delete a canvas in game mode)
	if (App->gameState == GameState::PLAY)
	{
		std::list<C_Canvas*>& activeCanvas = App->uiSystem->activeCanvas;
		for (std::list<C_Canvas*>::iterator it = activeCanvas.begin(); it != activeCanvas.end(); ++it)
		{
			if ((*it) == this)
			{
				activeCanvas.erase(it);
				break;
			}
		}
	}
	std::vector<C_Canvas*>& allCanvas = App->uiSystem->allCanvas;
	for (std::vector<C_Canvas*>::iterator it = allCanvas.begin(); it != allCanvas.end(); ++it)
	{
		if ((*it) == this)
		{
			allCanvas.erase(it);
			break;
		}
	}
}

bool C_Canvas::Update()
{
	OPTICK_CATEGORY("C_Canvas Update", Optick::Category::Update);

	//This will be world space only
	//TODO: This will be world space only
	if (IsActive())
		if (App->camera->GetCurrentCamera() != App->camera->masterCamera->GetComponent<C_Camera>())
			SetSize({ App->camera->GetCurrentCamera()->GetFrustum().NearPlaneWidth(), App->camera->GetCurrentCamera()->GetFrustum().NearPlaneHeight() });

	return true;
}

void C_Canvas::HandleInput()
{
	//if (!IsActive()) //poder fer tmbe un unable input??
		//return;

	if ((App->input->GetKey(SDL_SCANCODE_DOWN) == KeyState::KEY_DOWN || App->input->GetGameControllerAxis(3) == AxisState::POSITIVE_AXIS_DOWN))
	{
		for (int i = 0; i < uiElements.size(); ++i)
		{
			if (uiElements[i] == selectedUi)
			{
				int startingIndex = i;
				int j = startingIndex + 1;
				if (j >= uiElements.size())
					j = 0;
				for (j; j != startingIndex; ++j)
				{
					if (uiElements[j]->Interactuable())
					{
						selectedUi = uiElements[j];
						App->audio->aSourceUi->SetEvent("ui_navigate");
						App->audio->aSourceUi->PlayFx(App->audio->aSourceUi->GetEventId());
						break;
					}
					if (j == uiElements.size() -1)
						j = -1;
				}
				break;
			}
		}
	}
	else if ((App->input->GetKey(SDL_SCANCODE_UP) == KeyState::KEY_DOWN || App->input->GetGameControllerAxis(3) == AxisState::NEGATIVE_AXIS_DOWN))
	{
		for (int i = uiElements.size() - 1; i >= 0; --i)
		{
			if (uiElements[i] == selectedUi)
			{
				int startingIndex = i;
				int j = startingIndex - 1;
				if (j <= -1)
					j = uiElements.size() - 1;
				for (j; j != startingIndex; --j)
				{
					if (uiElements[j]->Interactuable()) 
					{
						selectedUi = uiElements[j];
						App->audio->aSourceUi->SetEvent("ui_navigate");
						App->audio->aSourceUi->PlayFx(App->audio->aSourceUi->GetEventId());
						break;
					}
					if (j == 0)
						j = uiElements.size();
				}
				break;
			}
		}
	}

	for (std::vector<C_UI*>::const_iterator it = uiElements.cbegin(); it != uiElements.cend(); ++it)
	{
		(*it)->HandleInput(&selectedUi);
	}
}

void C_Canvas::Draw2D(bool renderCanvas)
{
	if (renderCanvas)
	{
		glPushMatrix();
		glMultMatrixf((GLfloat*)&GetOwner()->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

		glLineWidth(2.0f);


		glBegin(GL_LINES);

		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);										// X Axis.
		glVertex2f(rect.x - rect.w / 2, rect.y + rect.h / 2);			glVertex2f(rect.x + rect.w / 2, rect.y + rect.h / 2);
		glVertex2f(rect.x + rect.w / 2, rect.y + rect.h / 2);			glVertex2f(rect.x + rect.w / 2, rect.y - rect.h / 2);
		glVertex2f(rect.x + rect.w / 2, rect.y - rect.h / 2);			glVertex2f(rect.x - rect.w / 2, rect.y - rect.h / 2);
		glVertex2f(rect.x - rect.w / 2, rect.y - rect.h / 2);			glVertex2f(rect.x - rect.w / 2, rect.y + rect.h / 2);

		glEnd();

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glLineWidth(1.0f);

		glPopMatrix();
	}
	for (std::vector<C_UI*>::reverse_iterator it = uiElements.rbegin(); it != uiElements.rend(); ++it) 
	{
		//passarli un rect2d del seu parent
		//Com es modifica el rect2d????
		(*it)->Draw2D();
	}
}

void C_Canvas::Draw3D(bool renderCanvas)
{
	if (renderCanvas)
	{
		glPushMatrix();
		glMultMatrixf((GLfloat*)&GetOwner()->GetComponent<C_Transform>()->GetWorldTransform().Transposed());

		glLineWidth(2.0f);

		glBegin(GL_LINES);

		glColor4f(1.0f, 0.0f, 0.0f, 1.0f);										// X Axis.
		glVertex2f(GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().x - rect.w / 2, GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().y + rect.h / 2);			glVertex2f(GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().x + rect.w / 2, GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().y + rect.h / 2);
		glVertex2f(GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().x + rect.w / 2, GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().y + rect.h / 2);			glVertex2f(GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().x + rect.w / 2, GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().y - rect.h / 2);
		glVertex2f(GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().x + rect.w / 2, GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().y - rect.h / 2);			glVertex2f(GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().x - rect.w / 2, GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().y - rect.h / 2);
		glVertex2f(GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().x - rect.w / 2, GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().y - rect.h / 2);			glVertex2f(GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().x - rect.w / 2, GetOwner()->GetComponent<C_Transform>()->GetLocalPosition().y + rect.h / 2);

		glEnd();

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glLineWidth(1.0f);

		glPopMatrix();
	}
}

bool C_Canvas::CleanUp()
{
	return true;
}

bool C_Canvas::SaveState(ParsonNode& root) const
{
	root.SetNumber("Type", (uint)GetType());

	ParsonNode canvas = root.SetNode("Canvas");

	canvas.SetBool("IsActive", IsActive());

	canvas.SetNumber("X", GetRect().x);
	canvas.SetNumber("Y", GetRect().y);
	canvas.SetNumber("W", GetRect().w);
	canvas.SetNumber("H", GetRect().h);

	return true;
}

bool C_Canvas::LoadState(ParsonNode& root)
{
	ParsonNode canvas = root.GetNode("Canvas");

	SetIsActive(canvas.GetBool("IsActive"));

	Rect2D r;

	r.x = canvas.GetNumber("X");
	r.y = canvas.GetNumber("Y");
	r.w = canvas.GetNumber("W");
	r.h = canvas.GetNumber("H");

	SetRect(r);
	
	//priority = canvas.GetInteger("Priority");

	return true;
}


void C_Canvas::RemoveUiElement(C_UI* element)
{
	for (std::vector<C_UI*>::iterator it = uiElements.begin(); it != uiElements.cend(); ++it)
	{
		if ((*it) == element)
		{
			uiElements.erase(it);
			break;
		}
	}
}

void C_Canvas::ResetUi()
{
	uiElements.clear();
	selectedUi = nullptr;
	cachedObjects = GetOwner()->childs;
	for (std::vector<GameObject*>::const_iterator it = cachedObjects.cbegin(); it != cachedObjects.cend(); ++it)
	{
		(*it)->GetUiComponents(uiElements);
	}
}

float2 C_Canvas::GetPosition() const
{
	return { rect.x, rect.y };
}

float2 C_Canvas::GetSize() const
{
	return { rect.w, rect.h };
}

Rect2D C_Canvas::GetRect() const
{
	return rect;
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

void C_Canvas::SetRect(const Rect2D& rect)
{
	this->rect.x = rect.x;
	this->rect.y = rect.y;
	this->rect.w = rect.w;
	this->rect.h = rect.h;
}

