#include "GameManager.h"
#include "Application.h"
#include "M_Window.h"
#include "M_Scene.h"
#include "C_Canvas.h"
#include "C_UI_Text.h"
#include "GameObject.h"
#include "MC_Time.h"

GameManager::GameManager(): Script()
{
}

GameManager::~GameManager()
{
}

void GameManager::Start()
{
	fpsCount = (C_UI_Text*)App->scene->GetGameObjectByName(fpsText.c_str())->GetComponent<C_UI_Text>();
}

void GameManager::Update()
{
	fpsCount->SetText(std::to_string(MC_Time::Real::GetFramesLastSecond()).c_str());
}
