#include "Application.h"

#include "M_Renderer3D.h"
#include "M_Camera3D.h"
#include "M_Window.h"
#include "M_UISystem.h"
#include "M_Scene.h"
#include "M_Input.h"

#include "GameObject.h"

#include "C_Camera.h"
#include "C_Transform.h"
#include "C_Canvas.h"
#include "C_UI_Button.h"
#include "C_UI_Image.h"

#include "Dependencies/glew/include/glew.h"
//#include "OpenGL.h"

#include "MemoryManager.h"

#include "JSONParser.h"


C_Canvas::C_Canvas(GameObject* owner) : Component(owner, ComponentType::CANVAS)
{
	isInvisible = false;

	App->uiSystem->priorityIterator++;
	priority = App->uiSystem->priorityIterator;

	// New one will allways be the input receiving canvas
	App->uiSystem->AddNewCanvas(this);
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

		if (App->camera->GetCurrentCamera() != App->camera->masterCamera->GetComponent<C_Camera>())
			SetSize({ App->camera->GetCurrentCamera()->GetFrustum().NearPlaneWidth(), App->camera->GetCurrentCamera()->GetFrustum().NearPlaneHeight() });
	}

	return ret;
}

void C_Canvas::Draw2D()
{
	if (App->renderer->GetRenderCanvas())
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
}

void C_Canvas::Draw3D()
{
	if (App->renderer->GetRenderCanvas())
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
	bool ret = true;

	App->uiSystem->DeleteCanvas(this);

	return ret;
}

bool C_Canvas::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("Type", (uint)GetType());

	ParsonNode canvas = root.SetNode("Canvas");

	canvas.SetBool("IsActive", IsActive());

	canvas.SetNumber("X", GetRect().x);
	canvas.SetNumber("Y", GetRect().y);
	canvas.SetNumber("W", GetRect().w);
	canvas.SetNumber("H", GetRect().h);
	canvas.SetBool("IsInvisible", isInvisible);

	if (App->uiSystem->inputCanvas == this)
		canvas.SetBool("IsInputCanvas", true);
	else
		canvas.SetBool("IsInputCanvas", false);

	canvas.SetInteger("Priority", priority);

	return ret;
}

bool C_Canvas::LoadState(ParsonNode& root)
{
	bool ret = true;

	ParsonNode canvas = root.GetNode("Canvas");

	SetIsActive(canvas.GetBool("IsActive"));

	Rect2D r;

	r.x = canvas.GetNumber("X");
	r.y = canvas.GetNumber("Y");
	r.w = canvas.GetNumber("W");
	r.h = canvas.GetNumber("H");

	SetRect(r);

	bool isInvis = canvas.GetBool("IsInvisible");
	SetIsInvisible(isInvis);

	if (canvas.GetBool("IsInputCanvas"))
		App->uiSystem->inputCanvas = this;
	
	//priority = canvas.GetInteger("Priority");

	return ret;
}


bool C_Canvas::CheckButtonStates()
{
	bool ret = false;

	if (selectedButton != nullptr)
	{
		if (selectedButton->GetState() == UIButtonState::RELEASED)
		{
			selectedButton->SetState(UIButtonState::HOVERED);
		}

		// Check for selected button getting pressed
		if (App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_DOWN || App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_REPEAT || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_DOWN || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_REPEAT)
		{
			selectedButton->OnPressed();
		}

		else if (App->input->GetKey(SDL_SCANCODE_RETURN) == KeyState::KEY_UP || App->input->GetGameControllerButton(0) == ButtonState::BUTTON_UP)
		{
			selectedButton->OnReleased();
		}


		//// Check for hovered button getting pressed
		//if (App->input->GetMouseButton(0) == KeyState::KEY_DOWN || App->input->GetMouseButton(0) == KeyState::KEY_REPEAT)
		//{
		//	hoveredButton->OnPressed();
		//}

		//else if (App->input->GetMouseButton(0) == KeyState::KEY_UP)
		//{
		//	hoveredButton->OnReleased();
		//}


		// Check inputs for controller/keyboard
		if (activeButtons.size() > 1)
		{
			bool prev = false;
			bool next = false;

			if ((App->input->GetKey(SDL_SCANCODE_UP) == KeyState::KEY_DOWN || App->input->GetGameControllerAxis(1) == AxisState::NEGATIVE_AXIS_DOWN) && !selectedButton->IsPressed())
			{
				for (std::vector<C_UI_Button*>::reverse_iterator buttonIt = activeButtons.rbegin(); buttonIt != activeButtons.rend(); buttonIt++)
				{
					if ((*buttonIt)->IsActive())
					{
						if ((*buttonIt)->GetState() == UIButtonState::HOVERED)
						{
							(*buttonIt)->SetState(UIButtonState::IDLE);
							prev = true;
						}
						else if (prev)
						{
							(*buttonIt)->SetState(UIButtonState::HOVERED);
							selectedButton = (*buttonIt);
							prev = false;
						}
					}
				}
				if (prev)
					selectedButton->SetState(UIButtonState::HOVERED);
			}

			if ((App->input->GetKey(SDL_SCANCODE_DOWN) == KeyState::KEY_DOWN || App->input->GetGameControllerAxis(1) == AxisState::POSITIVE_AXIS_DOWN) && !selectedButton->IsPressed())
			{
				for (std::vector<C_UI_Button*>::iterator buttonIt = activeButtons.begin(); buttonIt != activeButtons.end(); buttonIt++)
				{
					if ((*buttonIt)->IsActive())
					{
						if ((*buttonIt)->GetState() == UIButtonState::HOVERED)
						{
							(*buttonIt)->SetState(UIButtonState::IDLE);
							next = true;
						}
						else if (next)
						{
							(*buttonIt)->SetState(UIButtonState::HOVERED);
							selectedButton = (*buttonIt);
							next = false;
						}
					}
				}
				if (next)
					selectedButton->SetState(UIButtonState::HOVERED);
			}
		}

		//	!!!		Work In Progress	!!!

		// Checking for inputs with mouse
		/*if (activeButtons.size() > 1)
		{
			for (std::vector<C_UI_Button*>::const_iterator buttonIt = activeButtons.cbegin(); buttonIt != activeButtons.cend(); buttonIt++)
			{
				float2 mousePos = { (float)App->input->GetMouseX(), (float)App->input->GetMouseY() };
			}
		}*/
	}
	else if (activeButtons.size() > 1)
	{
		selectedButton = (*activeButtons.begin());
		selectedButton->SetState(UIButtonState::HOVERED);
	}

	return ret;
}


void C_Canvas::UpdateActiveButtons()
{
	if (activeButtons.size() < 2)
		return;

	// Create a new list and empty the other one into this one
	std::vector<C_UI_Button*> newButtonsList;
	while (!activeButtons.empty())
	{
		float y = -999;
		for (std::vector<C_UI_Button*>::iterator buttonIt = activeButtons.begin(); buttonIt != activeButtons.end(); buttonIt++)
		{
			if ((*buttonIt)->GetRect().y > y)
			{
				y = (*buttonIt)->GetRect().y;
				buttonIterator = (*buttonIt);
			}
		}
		for (std::vector<C_UI_Button*>::iterator buttonIt2 = activeButtons.begin(); buttonIt2 != activeButtons.end(); buttonIt2++)
		{
			if ((*buttonIt2) == buttonIterator)
			{
				newButtonsList.push_back(*buttonIt2);
				activeButtons.erase(buttonIt2);
				break;
			}
		}
	}
	activeButtons = newButtonsList;
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

bool C_Canvas::IsInvisible() const
{
	return isInvisible;
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

void C_Canvas::SetIsInvisible(const bool setTo)
{
	isInvisible = setTo;
}

