#include "GameApplication.h"
#include "M_GameManager.h"
#include "M_Renderer3D.h"

M_GameManager::M_GameManager(bool isActive) : Module("GameManager", isActive)
{

}

// Destructor
M_GameManager::~M_GameManager()
{

}

void M_GameManager::PostSceneRendering()
{
	GameApp->renderer->RenderFramebufferTexture();
}
