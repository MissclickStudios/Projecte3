#include "Application.h"
#include "Log.h"
#include "Profiler.h"

#include "M_ResourceManager.h"
#include "M_Scene.h"
#include "M_Camera3D.h"

#include "GameObject.h"

#include "R_Texture.h"

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
	activeCanvas.pop_front();
	return topCanvas;
}

/*void M_UISystem::DeleteCanvas(C_Canvas* canvas) Handle these on the destructor of C_Canvas !!!
{
	
}*/

const std::list<C_Canvas*>& M_UISystem::GetActiveCanvas() const
{
	return activeCanvas;
}

void M_UISystem::ClearActiveCanvas()
{
	activeCanvas.clear();
}
