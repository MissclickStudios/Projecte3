#include "Application.h"
#include "M_FileSystem.h"
#include "FileSystemDefinitions.h"
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
	//Check files exist (Maybe in another place)
	for (int i = 0; i < level.level1.size(); ++i) 
	{
		if (!App->fileSystem->Exists((ASSETS_SCENES_PATH + level.level1[i]).c_str())) 
		{
			level.level1.erase(level.level1.begin() + i);
			--i;
		}
	}
	for (int i = 0; i < level.level2.size(); ++i)
	{
		if (!App->fileSystem->Exists((ASSETS_SCENES_PATH + level.level1[i]).c_str()))
		{
			level.level2.erase(level.level2.begin() + i);
			--i;
		}
	}
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
