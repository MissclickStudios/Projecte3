#include <algorithm>
#include <chrono>
#include <random>
#include "Application.h"
#include "M_FileSystem.h"
#include "FileSystemDefinitions.h"
#include "M_Scene.h"
#include "M_Input.h"
#include "JSONParser.h"
#include "GameManager.h"
#include "Log.h"
#include "CoreDllHelpers.h"
#include "GameObject.h"
#include "M_ResourceManager.h"
#include "C_Transform.h"
#include "Player.h"

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
	if (enabled) 
	{
		if (mainMenuScene != App->scene->GetCurrentScene())
		{
			//Load Json state
			char* buffer = nullptr;
			App->fileSystem->Load(saveFileName, &buffer);
			ParsonNode jsonState(buffer);
			//release Json File
			CoreCrossDllHelpers::CoreReleaseBuffer(&buffer);
			currentLevel = jsonState.GetInteger("currentLevel");
			roomNum = jsonState.GetInteger("roomNum");
			level1.clear();
			//LEVEL2
			//level2.clear();
			ParsonArray levelArray = jsonState.GetArray("level1");
			level1.reserve(levelArray.size);
			for (int i = 0; i < levelArray.size;++i)
			{
				level1.emplace_back(levelArray.GetString(i));
			}
			//LEVEL2
			/*ParsonArray levelArray2 = jsonState.GetArray("level2");
			level2.reserve(levelArray2.size);
			for (int i = 0; i < levelArray2.size; ++i)
			{
				level2.emplace_back(levelArray2.GetString(i));
			}*/
			//TODO:Spawn player and everything on the level
			playerGameObject = App->resourceManager->LoadPrefab(playerPrefab.uid, App->scene->GetSceneRoot());
			GameObject* playerSpawn = App->scene->GetGameObjectByName(SpawnPointName.c_str());
			if (playerSpawn != nullptr && playerGameObject != nullptr) 
			{
				playerGameObject->transform->SetLocalPosition(playerSpawn->transform->GetLocalPosition());
				//playerTrans->SetLocalRotation(spawnTrans->GetLocalRotation());
			}
		}
	}
}

void GameManager::Start()
{
	if (enabled && mainMenuScene != App->scene->GetCurrentScene() && playerGameObject)
	{
		char* buffer = nullptr;
		App->fileSystem->Load(saveFileName, &buffer);
		ParsonNode jsonState(buffer);
		//release Json File
		CoreCrossDllHelpers::CoreReleaseBuffer(&buffer);
		playerScript = (Player*)playerGameObject->GetScript("Player");
		ParsonNode playerNode = jsonState.GetNode("player");
		playerScript->LoadState(playerNode);
		//Reset if HUB
		if (strstr(level1[0].c_str(), App->scene->GetCurrentScene()))
			playerScript->Reset();
	}
}

void GameManager::Update()
{
	// --- Room Generation
	if (enabled) 
	{
		HandleRoomGeneration();

		if (playerScript != nullptr && playerScript->moveState == PlayerState::DEAD_OUT) 
		{
			//playerScript->Reset();
			ReturnHub();
		}
	}

	//S'ha de fer alguna manera de avisar l'scene que volem canviar de scene pero no fer-ho imediatament ??? -> si
	//--
}

void GameManager::GenerateNewRun(bool fromMenu)
{
	if (enabled) 
	{
		//Check that the input room files exist
		for (int i = 0; i < level1.size(); ++i)
		{
			if(!strstr(level1[i].c_str(),ASSETS_SCENES_PATH))
				level1[i] = ASSETS_SCENES_PATH + level1[i];
			if (!App->fileSystem->Exists(level1[i].c_str()))
			{
				level1.erase(level1.begin() + i);
				--i;
			}
		}
		/*LEVEL2
		for (int i = 0; i < level2.size(); ++i)
		{
			if (!strstr(level2[i].c_str(), ASSETS_SCENES_PATH))
				level2[i] = ASSETS_SCENES_PATH + level2[i];
			if (!App->fileSystem->Exists(level2[i].c_str()))
			{
				level2.erase(level2.begin() + i);
				--i;
			}
		}*/

		if (fromMenu)
		{
			GenerateLevel(); //Nomes quan li donem a new game desde el main menu

			//TODO: Not HardCode the fixed rooms
			//TODO: inspector support adding fixed room
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "HUB.json").c_str()))
				//AddFixedRoom("InitialL1", 1, 1);
				level1.insert(level1.begin(), (std::string(ASSETS_SCENES_PATH) + "HUB.json"));
			//LEVEL2
			//if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "InitialL2.json").c_str()))
			//	AddFixedRoom("InitialL2", 2, 1);
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "InitialL1.json").c_str()))
				//AddFixedRoom("InitialL1", 1, 1);
				level1.insert(level1.begin() + 1, (std::string(ASSETS_SCENES_PATH) + "InitialL1.json"));
			//LEVEL2
			//if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "InitialL2.json").c_str()))
			//	AddFixedRoom("InitialL2", 2, 1);

			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "ShopL1.json").c_str()))
				//AddFixedRoom("ShopL1", 1, 4);
				level1.insert(level1.begin() + 4, (std::string(ASSETS_SCENES_PATH) + "ShopL1.json"));
			//LEVEL2
			//if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "ShopL2.json").c_str()))
			//	AddFixedRoom("ShopL2", 2, 4);

			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "BossL1.json").c_str()))
				//AddFixedRoom("BossL1", 1, 10);
				level1.push_back((std::string(ASSETS_SCENES_PATH) + "BossL1.json"));
			//LEVEL2
			//if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "BossL2.json").c_str()))
			//	AddFixedRoom("BossL2", 2, 10);
		}
		else
		{
			//LEVEL2 falta per fixed
			//remove fixed (harcoded)
			//level1.erase(level1.begin());
			//level1.erase(level1.begin() + 2);
			//level1.erase(level1.begin() + 6);
			for (int i = 0; i < level1.size(); ++i)
			{
				if (strstr(level1[i].c_str(), "HUB.json") || strstr(level1[i].c_str(), "InitialL1") || strstr(level1[i].c_str(), "ShopL1") || strstr(level1[i].c_str(), "BossL1"))
				{
					level1.erase(level1.begin() + i);
					--i;
				}
			}

			//randomize levels again
			GenerateLevel();

			//add fixed again
			//AddFixedRoom("InitialL1", 1, 1);
			//AddFixedRoom("BossL1", 1, 10);
			//AddFixedRoom("ShopL1", 1, 4);
			//AddFixedRoom("InitialL1", 1, 1);
			level1.insert(level1.begin(), (std::string(ASSETS_SCENES_PATH) + "HUB.json"));
			level1.insert(level1.begin() + 1, (std::string(ASSETS_SCENES_PATH) + "InitialL1.json"));
			//AddFixedRoom("BossL1", 1, 10);
			//AddFixedRoom("ShopL1", 1, 4);
			level1.insert(level1.begin() + 4, (std::string(ASSETS_SCENES_PATH) + "ShopL1.json"));
			level1.push_back((std::string(ASSETS_SCENES_PATH) + "BossL1.json"));
		}

		SaveManagerState();
	}
}

void GameManager::GenerateLevel()
{
	// get a time-based seed
	unsigned seed = std::chrono::system_clock::now()
		.time_since_epoch()
		.count();
	shuffle(level1.begin(), level1.end(), std::default_random_engine(seed));
	
	//LEVEL2
	/*seed = std::chrono::system_clock::now()
		.time_since_epoch()
		.count();
	shuffle(level2.begin(), level2.end(), std::default_random_engine(seed));*/
	
	//Fisher-Yates shuffle
	/*int size = level1.size();
	for (int i = 0; i < size - 1; ++i) 
	{
		int j = i + rand() % (size - i);
		std::swap(level1[i], level1[j]);
	}
	size = level2.size();
	for (int i = 0; i < size - 1; ++i)
	{
		int j = i + rand() % (size - i);
		std::swap(level2[i], level2[j]);
	}*/
	//std::random_shuffle(level1.begin(), level1.end());
	//std::random_shuffle(level2.begin(), level2.end());
}

void GameManager::GoNextRoom()
{
	if (enabled) 
	{
		if (currentLevel == 1)
		{
			if (!level1.empty())
			{
				if (roomNum < level1.size() - 1)
				{
					++roomNum;
					SaveManagerState();
					App->scene->ScriptChangeScene(level1[roomNum]);
				}

				else if (roomNum == level1.size() - 1)
				{
					LOG("[SCENE] Level Generator: End of the Game Reached!");
					//LEVEL2
					//InitiateLevel(2);
					//TODO:Win Condition
					ReturnHub();
				}
			}
		}
		//LEVEL2
		/*else if (currentLevel == 2)
		{
			if (!level2.empty())
			{
				if (roomNum < level2.size() - 1)
				{
					++roomNum;
					SaveManagerState();
					App->scene->ScriptChangeScene(level2[roomNum]);
				}
				else if (roomNum == level2.size() - 1)
				{
					LOG("[SCENE] Level Generator: End of the Game Reached!");

					InitiateLevel(1);
				}
			}
		}*/
	}
}

void GameManager::GoPreviousRoom()
{
	if (currentLevel == 1)
	{
		if (!level1.empty())
		{
			if (roomNum > 0)
			{
				--roomNum;
				SaveManagerState();
				App->scene->ScriptChangeScene(level1[roomNum]);
			}
			else
			{
				LOG("End of the level reached.");
			}
		}
	}
	//LEVEL2
	/*else if (currentLevel == 2)
	{
		if (!level2.empty())
		{
			if (roomNum > 0)
			{
				--roomNum;
				SaveManagerState();
				App->scene->ScriptChangeScene(level2[roomNum]);
			}
			else
			{
				LOG("End of the level reached.");
			}
		}
	}*/
}

void GameManager::InitiateLevel(int level)
{
	if (enabled)
	{
		if (level == 1)
		{
			if (!level1.empty())
			{
				roomNum = 0;
				currentLevel = 1;
				SaveManagerState();
				App->scene->ScriptChangeScene(level1[0]);
			}
		}
		//LEVEL2
		/*else if (level == 2)
		{
			if (!level2.empty())
			{
				roomNum = 0;
				currentLevel = 2;
				SaveManagerState();
				App->scene->ScriptChangeScene(level2[0]);
			}
		}*/
	}
}

void GameManager::Continue()
{
	if (enabled && App->fileSystem->Exists(saveFileName))
	{
		char* buffer = nullptr;
		App->fileSystem->Load(saveFileName, &buffer);
		ParsonNode jsonState(buffer);
		//release Json File
		CoreCrossDllHelpers::CoreReleaseBuffer(&buffer);
		currentLevel = jsonState.GetInteger("currentLevel");
		roomNum = jsonState.GetInteger("roomNum");
		level1.clear();
		//LEVEL2
		//level2.clear();
		ParsonArray levelArray = jsonState.GetArray("level1");
		level1.reserve(levelArray.size);
		for (int i = 0; i < levelArray.size; ++i)
		{
			level1.emplace_back(levelArray.GetString(i));
		}
		//LEVEL2
		/*ParsonArray levelArray2 = jsonState.GetArray("level2");
		level2.reserve(levelArray2.size);
		for (int i = 0; i < levelArray2.size; ++i)
		{
			level2.emplace_back(levelArray2.GetString(i));
		}*/
		//TODO:Spawn player and everything on the level
		if (currentLevel == 1)
			App->scene->ScriptChangeScene(level1[roomNum]);
		//LEVEL2
		//else if (currentLevel == 2)
		//	App->scene->ScriptChangeScene(level1[roomNum]);
	}
}

void GameManager::ReturnHub()
{
	currentLevel = 1;
	roomNum = 0;
	GenerateNewRun(false);
	SaveManagerState();
	App->scene->ScriptChangeScene(level1[0]);
}

void GameManager::ReturnToMainMenu()
{
	std::string menuPath = ASSETS_SCENES_PATH + mainMenuScene + ".json";
	App->scene->ScriptChangeScene(menuPath.c_str());
}

void GameManager::AddFixedRoom(std::string name, int level, int position)
{
	int newPosition = position - 1;
	std::string roomPath = ASSETS_SCENES_PATH + name + ".json";

	if (App->fileSystem->Exists(roomPath.c_str()))
	{
		std::string tempRoom;
		std::string tempRoom2;

		if (level == 1)
		{
			level1.resize(level1.size() + 1);

			if (level1.size() <= newPosition)
			{
				newPosition = level1.size();
			}
			for (int i = 0; i <= level1.size(); i++)
			{
				if (i == newPosition)
				{
					if (i == level1.size())
					{
						level1[i - 1] = roomPath;
					}
					else
					{
						tempRoom = level1[i + 1];
						level1[i + 1] = level1[i];
						level1[i] = roomPath;
					}
				}
				else if (i > newPosition && i < level1.size())
				{
					tempRoom2 = level1[i];
					level1[i] = tempRoom;
					if (i + 1 != level1.size()) tempRoom = level1[i + 1];
					tempRoom = tempRoom2;
				}

			}
		}

		else if (level == 2)
		{
			level2.resize(level2.size() + 1);

			if (level2.size() <= newPosition)
			{
				newPosition = level2.size();
			}
			for (int i = 0; i <= level2.size(); i++)
			{
				if (i == newPosition)
				{
					if (i == level2.size())
					{
						level2[i - 1] = roomPath;
					}
					else
					{
						tempRoom = level2[i + 1];
						level2[i + 1] = level2[i];
						level2[i] = roomPath;
					}
				}
				else if (i > newPosition && i < level2.size())
				{
					tempRoom2 = level2[i];
					level2[i] = tempRoom;
					if (i + 1 != level2.size()) tempRoom = level2[i + 1];
					tempRoom = tempRoom2;
				}

			}
		}
	}
}

void GameManager::HandleRoomGeneration()
{
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KeyState::KEY_REPEAT)
	{
		if (App->input->GetKey(SDL_SCANCODE_KP_6) == KeyState::KEY_DOWN)
		{

			GoNextRoom();

			/*if(currentLevel == 1)
			{

				(roomNum < level1.size() - 1) ? GoNextRoom() : LOG("[SCENE] Level Generator: End of the Level Reached!");

				(roomNum == level1.size()) ? InitiateLevel(2) : LOG("[SCENE] Level Generator: Begin Level 2 ");
			}
			else if (currentLevel == 2)
			{
				(roomNum < level2.size() - 1) ? GoNextRoom() : LOG("[SCENE] Level Generator: End of the Game Reached!");
<<<<<<< Updated upstream
			}*/

		}
		if (App->input->GetKey(SDL_SCANCODE_KP_4) == KeyState::KEY_DOWN)
		{
			if (currentLevel == 1)
			{
				(roomNum > 0) ? GoPreviousRoom() : LOG("[SCENE] Level Generator: Begin of the Level Reached!");

			}
			//LEVEL2
			/*else if (currentLevel == 2)
			{
				(roomNum > 0) ? GoPreviousRoom() : LOG("[SCENE] Level Generator: End of the Level Reached!");

				(roomNum == 0) ? InitiateLevel(1) : LOG("[SCENE] Level Generator: Begin Level 1 ");

			}*/
		}
	}
}

void GameManager::SaveManagerState()
{
	ParsonNode jsonState;
	//release Json File
	jsonState.SetInteger("currentLevel",currentLevel);
	jsonState.SetInteger("roomNum",roomNum);
	ParsonArray levelArray = jsonState.SetArray("level1");
	for (int i = 0; i < level1.size(); ++i)
	{
		levelArray.SetString(level1[i].c_str());
	}
	//LEVEL2
	/*ParsonArray levelArray2 = jsonState.SetArray("level2");
	for (int i = 0; i < level2.size(); ++i)
	{
		levelArray2.SetString(level2[i].c_str());
	}*/
	if (playerGameObject) 
	{
		ParsonNode playerNode = jsonState.SetNode("player");
		playerScript->SaveState(playerNode);
	}
	char* buffer = nullptr;
	jsonState.SerializeToFile(saveFileName, &buffer);
	CoreCrossDllHelpers::CoreReleaseBuffer(&buffer);
}

GameManager* CreateGameManager() {
	GameManager* script = new GameManager();
	INSPECTOR_CHECKBOX_BOOL(script->enabled);
	INSPECTOR_STRING(script->mainMenuScene);
	INSPECTOR_STRING(script->SpawnPointName);
	INSPECTOR_VECTOR_STRING(script->level1);
	//LEVEL2
	//INSPECTOR_VECTOR_STRING(script->level2);
	INSPECTOR_PREFAB(script->playerPrefab);
	return script;
}
