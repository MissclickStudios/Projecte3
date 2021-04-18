#include "GameManager.h"

GameManager::GameManager(): Script()
{
}

GameManager::~GameManager()
{
}

void GameManager::Awake()
{
	/*level.AddFixedRoom("Start",1 ,1);
	level.AddFixedRoom("Boss",1 ,15);*/

	//Load de la primera scene?
}

void GameManager::Update()
{
	// --- Room Generation
	level.HandleRoomGeneration();
	//S'ha de fer alguna manera de avisar l'scene que volem canviar de scene pero no fer-ho imediatament ??? -> si
	//--
}

void GameManager::GenerateNewRun()
{
	level.GenerateLevel(); //Nomes quan li donem a new game desde el main menu

	level.AddFixedRoom("InitialL1", 1, 1); //TODO: inspector support adding fixed room
	level.AddFixedRoom("InitialL2", 2, 1);

	level.AddFixedRoom("BossL1", 1, 10);
	level.AddFixedRoom("BossL2", 2, 10);

	level.AddFixedRoom("ShopL1", 1, 4);
	level.AddFixedRoom("ShopL2", 2, 4);
}

GameManager* CreateGameManager() {
	GameManager* script = new GameManager();
	INSPECTOR_VECTOR_STRING(script->level.level1);
	INSPECTOR_VECTOR_STRING(script->level.level2);
	return script;
}
