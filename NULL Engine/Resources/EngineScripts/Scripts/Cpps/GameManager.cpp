#include <algorithm>
#include <chrono>
#include <random>

#include "FileSystemDefinitions.h"
#include "JSONParser.h"
#include "Log.h"
#include "CoreDllHelpers.h"

#include "Application.h"

#include "M_FileSystem.h"
#include "M_ResourceManager.h"
#include "M_Scene.h"
#include "M_Input.h"

#include "GameObject.h"

#include "C_Transform.h"

#include "GameManager.h"
#include "Player.h"
#include "Gate.h"

#include "Items.h"

GameManager::GameManager(): Script()
{
}

GameManager::~GameManager()
{
	while (chestItemPool.size())
	{
		delete * chestItemPool.begin();
		chestItemPool.erase(chestItemPool.begin());
	}
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
			level1Ruins.clear();
			ParsonArray levelArray = jsonState.GetArray("level1");
			level1.reserve(levelArray.size);
			for (int i = 0; i < levelArray.size;++i)
			{
				level1.emplace_back(levelArray.GetString(i));
			}
			//LEVEL2
			ParsonArray levelArray2 = jsonState.GetArray("level1Ruins");
			level1Ruins.reserve(levelArray2.size);
			for (int i = 0; i < levelArray2.size; ++i)
			{
				level1Ruins.emplace_back(levelArray2.GetString(i));
			}
			//TODO:Spawn player and everything on the level
			GameObject* playerSpawn = App->scene->GetGameObjectByName(SpawnPointName.c_str());
			if (playerSpawn != nullptr) 
			{
				spawnPoint = playerSpawn->transform->GetLocalPosition();
				playerGameObject = App->scene->InstantiatePrefab(playerPrefab.uid, App->scene->GetSceneRoot(), spawnPoint,Quat::identity);
			}

			groguGameObject = App->resourceManager->LoadPrefab(groguPrefab.uid, App->scene->GetSceneRoot());
			if (playerSpawn != nullptr && groguGameObject != nullptr)
			{
				float3 offset = { 0,0,7 };
				spawnPoint = playerSpawn->transform->GetLocalPosition() + offset;
				groguGameObject->transform->SetLocalPosition(spawnPoint);
			}

			backtrackTimer.Start();
			if (backtrack.size() != 0)
				backtrack.clear();

			// Clear the vector if it has data
			while (chestItemPool.size())
			{
				delete* chestItemPool.begin();
				chestItemPool.erase(chestItemPool.begin());
			}
			//Load Json state
			char* itemBuffer = nullptr;
			App->fileSystem->Load("ChestItemPool.json", &itemBuffer);
			ParsonNode itemFile(itemBuffer);
			//release Json File
			CoreCrossDllHelpers::CoreReleaseBuffer(&itemBuffer);
			ParsonArray itemArray = itemFile.GetArray("Items");
			for (uint i = 0; i < itemArray.size; ++i)
			{
				ParsonNode itemNode = itemArray.GetNode(i);
				if (!itemNode.NodeIsValid())
					break;

				std::string name = itemNode.GetString("Name");
				std::string description = itemNode.GetString("Description");
				int price = itemNode.GetInteger("Price");
				ItemRarity rarity = (ItemRarity)itemNode.GetInteger("Rarity");
				int min = itemNode.GetInteger("Min");
				int max = itemNode.GetInteger("Max");
				float power = itemNode.GetNumber("Power");
				float duration = itemNode.GetInteger("Duration");
				float chance = itemNode.GetInteger("Chance");
				std::string texturePath = itemNode.GetString("Texture Path");
				chestItemPool.emplace_back(new ItemData(name, description, price, rarity, power, duration, chance, min, max, texturePath));
			}
		}
	}

	GameObject* tmp = App->scene->GetGameObjectByName(gateName.c_str());

	if (tmp != nullptr)
	{
		gate = (Gate*)tmp->GetScript("Gate");
	}
}

void GameManager::Start()
{
	//find all enemies
	std::vector<GameObject*>* objects = App->scene->GetGameObjects();
	for (auto go = objects->begin(); go != objects->end(); ++go)
	{
		Entity* entity = (Entity*)GetObjectScript((*go), ObjectType::ENTITY);
		if (entity != nullptr && entity->type != EntityType::PLAYER && entity->type != EntityType::GROGU)
		{
			enemies.push_back(entity);
		}
	}


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
	BackTrackUpdate();

	// --- Room Generation
	if (enabled) 
	{
		HandleRoomGeneration();

		if (playerScript != nullptr && playerScript->moveState == PlayerState::DEAD_OUT) 
		{
			//playerScript->Reset();
			//ReturnHub();
			App->scene->ScriptChangeScene(loseSceneName.c_str());
		}
	}

	GateUpdate(); //Checks if gate should be unlocked

	//S'ha de fer alguna manera de avisar l'scene que volem canviar de scene pero no fer-ho imediatament ??? -> si
	//--
}

void GameManager::OnCollisionEnter(GameObject* object)
{
	if (object == nullptr)
		return;

	if (object == playerGameObject)
	{
		if (playerScript != nullptr)
		{
			playerScript->TakeDamage(0.5f);
			move = true;
		}
	}
	else
	{
		Entity* entity = (Entity*)GetObjectScript(object, ObjectType::ENTITY);
		if (entity != nullptr)
			entity->TakeDamage(9999999999.0f);
	}
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
		//LEVEL2
		for (int i = 0; i < level1Ruins.size(); ++i)
		{
			if (!strstr(level1Ruins[i].c_str(), ASSETS_SCENES_PATH))
				level1Ruins[i] = ASSETS_SCENES_PATH + level1Ruins[i];
			if (!App->fileSystem->Exists(level1Ruins[i].c_str()))
			{
				level1Ruins.erase(level1Ruins.begin() + i);
				--i;
			}
		}

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
				level1.insert(level1.begin() + 6, (std::string(ASSETS_SCENES_PATH) + "ShopL1.json"));
			//LEVEL2
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "Shop_Ruins.json").c_str()))
				level1Ruins.insert(level1Ruins.begin() + 3, (std::string(ASSETS_SCENES_PATH) + "Shop_Ruins.json"));

			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "BossL1.json").c_str()))
				//AddFixedRoom("BossL1", 1, 10);
				level1.push_back((std::string(ASSETS_SCENES_PATH) + "BossL1.json"));
			//LEVEL2
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "Boss_Ruins.json").c_str()))
				//AddFixedRoom("BossL2", 2, 10);
				level1Ruins.push_back((std::string(ASSETS_SCENES_PATH) + "Boss_Ruins.json"));
			//LEVEL2
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "WinScene.json").c_str()))
				level1Ruins.push_back((std::string(ASSETS_SCENES_PATH) + "WinScene.json"));
		}
		else
		{
			//Remove fixed before randomizing
			for (int i = 0; i < level1.size(); ++i)
			{
				if (strstr(level1[i].c_str(), "HUB.json") || strstr(level1[i].c_str(), "InitialL1") || strstr(level1[i].c_str(), "ShopL1") || strstr(level1[i].c_str(), "BossL1"))
				{
					level1.erase(level1.begin() + i);
					--i;
				}
			}
			for (int i = 0; i < level1Ruins.size(); ++i)
			{
				if (strstr(level1Ruins[i].c_str(), "Shop_Ruins") || strstr(level1Ruins[i].c_str(), "Boss_Ruins") || strstr(level1Ruins[i].c_str(), "WinScene"))
				{
					level1Ruins.erase(level1Ruins.begin() + i);
					--i;
				}
			}

			//randomize levels again
			GenerateLevel();

			//add fixed again
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "HUB.json").c_str()))
				level1.insert(level1.begin(), (std::string(ASSETS_SCENES_PATH) + "HUB.json"));
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "InitialL1.json").c_str()))
				level1.insert(level1.begin() + 1, (std::string(ASSETS_SCENES_PATH) + "InitialL1.json"));
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "ShopL1.json").c_str()))
				level1.insert(level1.begin() + 6, (std::string(ASSETS_SCENES_PATH) + "ShopL1.json"));
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "BossL1.json").c_str()))
				level1.push_back((std::string(ASSETS_SCENES_PATH) + "BossL1.json"));

			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "Shop_Ruins.json").c_str()))
				level1Ruins.insert(level1Ruins.begin() + 3, (std::string(ASSETS_SCENES_PATH) + "Shop_Ruins.json"));
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "Boss_Ruins.json").c_str()))
				level1Ruins.push_back((std::string(ASSETS_SCENES_PATH) + "Boss_Ruins.json"));
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "WinScene.json").c_str()))
				level1Ruins.push_back((std::string(ASSETS_SCENES_PATH) + "WinScene.json"));
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
	if (l1Easy >= 0 && l1Easy < level1.size() && l1Intermediate > l1Easy && l1Intermediate < level1.size()) 
	{
		shuffle(level1.begin(), level1.begin() + l1Easy, std::default_random_engine(seed));
		shuffle(level1.begin() + l1Easy + 1, level1.begin() + l1Intermediate, std::default_random_engine(seed));
		shuffle(level1.begin() + l1Intermediate + 1, level1.end(), std::default_random_engine(seed));
	}
	else
		shuffle(level1.begin(), level1.end(), std::default_random_engine(seed));
	
	//LEVEL2
	seed = std::chrono::system_clock::now()
		.time_since_epoch()
		.count();
	if (RuinsEasy >= 0 && RuinsEasy < level1Ruins.size() && RuinsIntermediate > RuinsEasy && RuinsIntermediate < level1Ruins.size()) 
	{
		shuffle(level1Ruins.begin(), level1Ruins.begin() + RuinsEasy, std::default_random_engine(seed));
		shuffle(level1Ruins.begin() + RuinsEasy + 1, level1Ruins.begin() + RuinsIntermediate, std::default_random_engine(seed));
		shuffle(level1Ruins.begin() + RuinsIntermediate + 1, level1Ruins.end(), std::default_random_engine(seed));
	}
	else
		shuffle(level1Ruins.begin(), level1Ruins.end(), std::default_random_engine(seed));

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
					InitiateLevel(2);
					//TODO: Win Condition poder posar 
					//ReturnHub();
				}
			}
		}
		//LEVEL2
		else if (currentLevel == 2)
		{
			if (!level1Ruins.empty())
			{
				if (roomNum < level1Ruins.size() - 1)
				{
					++roomNum;
					SaveManagerState();
					App->scene->ScriptChangeScene(level1Ruins[roomNum]);
				}
				else if (roomNum == level1Ruins.size() - 1)
				{
					LOG("[SCENE] Level Generator: End of the Game Reached!");

					InitiateLevel(1);
				}
			}
		}
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
	else if (currentLevel == 2)
	{
		if (!level1Ruins.empty())
		{
			if (roomNum > 0)
			{
				--roomNum;
				SaveManagerState();
				App->scene->ScriptChangeScene(level1Ruins[roomNum]);
			}
			else
			{
				LOG("End of the level reached.");
			}
		}
	}
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
		else if (level == 2)
		{
			if (!level1Ruins.empty())
			{
				roomNum = 0;
				currentLevel = 2;
				SaveManagerState();
				App->scene->ScriptChangeScene(level1Ruins[0]);
			}
		}
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
		level1Ruins.clear();
		ParsonArray levelArray = jsonState.GetArray("level1");
		level1.reserve(levelArray.size);
		for (int i = 0; i < levelArray.size; ++i)
		{
			level1.emplace_back(levelArray.GetString(i));
		}
		//LEVEL2
		ParsonArray levelArray2 = jsonState.GetArray("level1Ruins");
		level1Ruins.reserve(levelArray2.size);
		for (int i = 0; i < levelArray2.size; ++i)
		{
			level1Ruins.emplace_back(levelArray2.GetString(i));
		}
		//TODO:Spawn player and everything on the level
		if (currentLevel == 1)
			App->scene->ScriptChangeScene(level1[roomNum]);
		//LEVEL2
		else if (currentLevel == 2)
			App->scene->ScriptChangeScene(level1Ruins[roomNum]);
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
			level1Ruins.resize(level1Ruins.size() + 1);

			if (level1Ruins.size() <= newPosition)
			{
				newPosition = level1Ruins.size();
			}
			for (int i = 0; i <= level1Ruins.size(); i++)
			{
				if (i == newPosition)
				{
					if (i == level1Ruins.size())
					{
						level1Ruins[i - 1] = roomPath;
					}
					else
					{
						tempRoom = level1Ruins[i + 1];
						level1Ruins[i + 1] = level1Ruins[i];
						level1Ruins[i] = roomPath;
					}
				}
				else if (i > newPosition && i < level1Ruins.size())
				{
					tempRoom2 = level1Ruins[i];
					level1Ruins[i] = tempRoom;
					if (i + 1 != level1Ruins.size()) tempRoom = level1Ruins[i + 1];
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
			//TODO:
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
			else if (currentLevel == 2)
			{
				(roomNum > 0) ? GoPreviousRoom() : LOG("[SCENE] Level Generator: End of the Level Reached!");

				(roomNum == 0) ? InitiateLevel(1) : LOG("[SCENE] Level Generator: Begin Level 1 ");

			}
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
	ParsonArray levelArray2 = jsonState.SetArray("level1Ruins");
	for (int i = 0; i < level1Ruins.size(); ++i)
	{
		levelArray2.SetString(level1Ruins[i].c_str());
	}
	if (playerGameObject) 
	{
		ParsonNode playerNode = jsonState.SetNode("player");
		playerScript->SaveState(playerNode);
	}
	char* buffer = nullptr;
	jsonState.SerializeToFile(saveFileName, &buffer);
	CoreCrossDllHelpers::CoreReleaseBuffer(&buffer);
}

void GameManager::BackTrackUpdate()
{
	if (playerScript != nullptr)
	{
		if (backtrackTimer.ReadSec() >= backtrackDuration)
		{
			if (playerScript != nullptr && playerScript->moveState != PlayerState::DASH && playerScript->IsGrounded())
			{
				if (backtrack.size() >= BACKTRACK)
					backtrack.erase(backtrack.begin());
				backtrack.push_back(playerScript->gameObject->transform->GetLocalPosition());
			}

			backtrackTimer.Start();
		}
		if (move)
		{
			move = false;
			float3 point = spawnPoint;
			for (int i = backtrack.size() - 1; i >= 0; --i)
			{
				if (i == 0)
					break;

				float current = backtrack[i].y * 1000;
				int currentRounded = (int)(backtrack[i].y * 1000);
				if (current >= (float)currentRounded)
					current = (float)currentRounded;
				else
					current = (float)(currentRounded - 1);

				float past = backtrack[i - 1].y * 1000;
				int pastRounded = (int)(backtrack[i - 1].y * 1000);
				if (past >= (float)pastRounded)
					past = (float)pastRounded;
				else
					past = (float)(pastRounded - 1);

				if (current != past)
					continue;
				point = backtrack[i];
				break;
			}
			playerScript->ChangePosition(point);
		}
	}
}

void GameManager::GateUpdate()
{
	if(gate != nullptr)
		if (gate->isLocked)
		{
			if (enemies.size() <= 0)
			{
				gate->Unlock();
				return;
			}

			for (auto enemy = enemies.begin(); enemy != enemies.end(); ++enemy)
			{
				if ((*enemy)->health > 0)
					return;
			}

			gate->Unlock();
		}
}

GameManager* CreateGameManager() {
	GameManager* script = new GameManager();
	INSPECTOR_CHECKBOX_BOOL(script->enabled);
	INSPECTOR_STRING(script->mainMenuScene);
	INSPECTOR_STRING(script->SpawnPointName);
	INSPECTOR_STRING(script->loseSceneName);
	INSPECTOR_INPUT_INT(script->l1Easy);
	INSPECTOR_INPUT_INT(script->l1Intermediate);
	INSPECTOR_VECTOR_STRING(script->level1);
	INSPECTOR_INPUT_INT(script->RuinsEasy);
	INSPECTOR_INPUT_INT(script->RuinsIntermediate);
	INSPECTOR_VECTOR_STRING(script->level1Ruins);
	INSPECTOR_PREFAB(script->playerPrefab);
	INSPECTOR_PREFAB(script->groguPrefab);
	return script;
}
