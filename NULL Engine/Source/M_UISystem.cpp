#include "Application.h"
#include "Log.h"
#include "Profiler.h"

#include "M_ResourceManager.h"
#include "M_Scene.h"
#include "M_Camera3D.h"

#include "GameObject.h"

#include "R_Texture.h"

#include "C_UI.h"
#include "C_Canvas.h"
#include "C_Camera.h"
#include "C_Transform.h"

#include "M_UISystem.h"

#include "MemoryManager.h"

#include "JSONParser.h"

M_UISystem::M_UISystem(bool isActive) : Module("UISystem", isActive)
{

}

// Destructor
M_UISystem::~M_UISystem()
{

}

UpdateStatus M_UISystem::Update(float dt)
{
	OPTICK_CATEGORY("M_UISystem Update", Optick::Category::Module);
	for (std::list<C_Canvas*>::reverse_iterator rit = activeCanvas.rbegin(); rit != activeCanvas.rend(); ++rit)
	{
		if ((*rit)->IsActive())
		{
			//This code is very ugly
			//We need event system to react when a gameObject is deleted or a child order changed
			std::vector<GameObject*>& children = (*rit)->GetOwner()->childs;
			if (children.size() != (*rit)->cachedObjects.size())
				(*rit)->ResetUi();
			else
			{
				for (int i = 0; i < children.size(); ++i)
				{
					if (children[i] != (*rit)->cachedObjects[i]) 
					{
						(*rit)->ResetUi();
						break;
					}
				}
			}
		}
	}
	if (App->gameState == GameState::PLAY && !activeCanvas.empty())
		activeCanvas.front()->HandleInput();

	return UpdateStatus::CONTINUE;
}

// Called before quitting
bool M_UISystem::CleanUp()
{
	activeCanvas.clear();
	return true;
}

bool M_UISystem::LoadConfiguration(ParsonNode& root)
{
	return true;
}

bool M_UISystem::SaveConfiguration(ParsonNode& root) const
{
	return true;
}

void M_UISystem::PushCanvas(C_Canvas* canvas)
{
	if (canvas) 
	{
		canvas->ResetUi();
		activeCanvas.push_front(canvas);
	}
}

C_Canvas* M_UISystem::PopCanvas()
{
	C_Canvas* topCanvas = activeCanvas.front();
	topCanvas->cachedObjects.clear();
	activeCanvas.pop_front();
	return topCanvas;
}

void M_UISystem::RemoveActiveCanvas(C_Canvas* canvas)
{
	for (std::list<C_Canvas*>::iterator it = activeCanvas.begin(); it != activeCanvas.end(); ++it)
	{
		if ((*it) == canvas)
		{
			(*it)->cachedObjects.clear();
			activeCanvas.erase(it);
			break;
		}
	}
}

const std::list<C_Canvas*>& M_UISystem::GetActiveCanvas() const
{
	return activeCanvas;
}

const std::vector<C_Canvas*>& M_UISystem::GetAllCanvas() const
{
	return allCanvas;
}

void M_UISystem::ClearActiveCanvas()
{
	activeCanvas.clear();
}

void M_UISystem::ReorderCanvasChildren()
{
	for (std::vector<C_Canvas*>::const_iterator it = allCanvas.cbegin(); it != allCanvas.cend(); ++it)
	{
		std::vector<GameObject*>& children = (*it)->GetOwner()->childs;
		int index = -1;
		for (int i = 0; i < children.size(); ++i)
		{
			if (children[i]->GetUiComponent() != nullptr)
			{
				while (index != i)
				{
					index = (*children[i]->GetUiComponent()).childOrder;
					GameObject* temp = children[index];
					children[index] = children[i];
					children[i] = temp;
				}
			}
		}
	}
}

void M_UISystem::SaveCanvasChildrenOrder()
{
	for (std::vector<C_Canvas*>::const_iterator it = allCanvas.cbegin(); it != allCanvas.cend(); ++it)
	{
		std::vector<GameObject*>& children = (*it)->GetOwner()->childs;
		for (int i = 0; i < children.size(); ++i)
		{
			std::vector<C_UI*>uiComponents;
			children[i]->SetUiChildOrder(i);
		}
	}
}
