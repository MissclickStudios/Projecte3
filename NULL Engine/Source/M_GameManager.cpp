#include "GameApplication.h"
#include "M_GameManager.h"
#include "M_Renderer3D.h"
#include "M_ScriptManager.h"
#include "Time.h"

M_GameManager::M_GameManager(bool isActive) : Module("GameManager", isActive)
{

}

// Destructor
M_GameManager::~M_GameManager()
{

}

bool M_GameManager::Init(ParsonNode& config)
{
	
	return true;
}

bool M_GameManager::Start()
{
	Time::Game::Play();
	GameApp->gameState = GameState::PLAY;
	GameApp->renderer->SetRenderWorldGrid(false);
	GameApp->renderer->SetRenderWorldAxis(false);
	GameApp->scriptManager->InitScripts();
	return true;
}

void M_GameManager::PostSceneRendering()
{
	GameApp->renderer->RenderFramebufferTexture();
}
