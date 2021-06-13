#include "FileSystemDefinitions.h"
#include "JSONParser.h"
#include "Log.h"
#include "CoreDllHelpers.h"

#include "Application.h"

#include "M_FileSystem.h"
#include "M_ResourceManager.h"
#include "M_Scene.h"
#include "M_Input.h"
#include "M_Audio.h"

#include "GameObject.h"

#include "C_AudioSource.h"
#include "C_Transform.h"
#include "C_UI_Text.h"
#include "C_AudioSource.h"
#include "C_RigidBody.h"

#include "GameManager.h"
#include "DialogManager.h"
#include "Player.h"
#include "Gate.h"
#include "CameraMovement.h"

#include "Items.h"
#include "Random.h"
#include "MC_Time.h"

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
	if (App->scene->creditsMainMenu)
	{
		HandleBackgroundMusic();
		return;
	}

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
			cameraShake = jsonState.GetBool("cameraShake");
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
			//itemsArmorer
			LoadArmorerItemLvl(jsonState);

			//Load story & dialogs
			storyDialogState.Load(&jsonState);

			runStats.Load(&jsonState);

			//TODO:Spawn player and everything on the level
			GameObject* playerSpawn = App->scene->GetGameObjectByName(SpawnPointName.c_str());
			if (playerSpawn != nullptr) 
			{
				spawnPoint = playerSpawn->transform->GetLocalPosition();
				playerGameObject = App->scene->InstantiatePrefab(playerPrefab.uid, App->scene->GetSceneRoot(), spawnPoint,Quat::identity);
			}

			if (storyDialogState.defeatedIG11FirstTime && playerGameObject != nullptr)
			{
				groguGameObject = App->scene->InstantiatePrefab(groguPrefab.uid, App->scene->GetSceneRoot(), playerSpawn->transform->GetLocalPosition(), Quat::identity, playerSpawn->transform->GetWorldPosition());

			}

			if (playerSpawn != nullptr && groguGameObject != nullptr && storyDialogState.defeatedIG11FirstTime)
			{
				float3 offset = { 0,0,7 };
				spawnPoint = playerSpawn->transform->GetLocalPosition() + offset;
				groguGameObject->transform->SetLocalPosition(spawnPoint);
			}

			backtrackTimer.Start();
			if (backtrack.size() != 0)
				backtrack.clear();

			LoadItemPool(chestItemPool, "ChestItemPool.json");
			LoadItemPool(shopItemPool, "ShopItemPool.json");
			LoadItemPool(hubItemPool, "HubItemPool.json");
		}

		HandleBackgroundMusic();
	}

	GameObject* tmp = App->scene->GetGameObjectByName("DialogCanvas"); 

	if(tmp!=nullptr)
		dialogManager = (DialogManager*)tmp->GetScript("DialogManager");

	tmp = App->scene->GetGameObjectByName(gateName.c_str());
	if (tmp != nullptr)
	{
		gate = (Gate*)tmp->GetScript("Gate");
	}

	
}

void GameManager::Start()
{
	if (App->scene->creditsMainMenu)
		return;

	//find all enemies
	std::vector<GameObject*>* objects = App->scene->GetGameObjects();
	for (auto go = objects->begin(); go != objects->end(); ++go)
	{
		Entity* entity = (Entity*)GetObjectScript((*go), ObjectType::ENTITY);
		if (entity != nullptr && entity->type != EntityType::PLAYER && entity->type != EntityType::GROGU)
		{
			enemies.push_back(std::make_pair(false, entity));
		}
	}

	//Load Player state
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

	//Camera cutscenes movement
	cameraGameObject = App->scene->GetGameObjectByName(cameraName.c_str());

	if(cameraGameObject != nullptr)
		cameraScript = (CameraMovement*)cameraGameObject->GetScript("CameraMovement");

	if (strcmp(App->scene->GetCurrentScene(), levelNames.winScene.c_str()) == 0) //Win screen
	{
		SetUpWinScreen();
	}

	if (strcmp(App->scene->GetCurrentScene(), levelNames.winScene.c_str()) == 0 || strcmp(App->scene->GetCurrentScene(), levelNames.loseScene.c_str()) == 0 || 
		strcmp(App->scene->GetCurrentScene(), "Credits") == 0) //Lock mando (He is flying in the scene)
	{
		playerScript->DisableInput(); //Pause mando so he doesn't produce any sound or movement (Silence him o_o)
		playerScript->gameObject->GetComponent<C_RigidBody>()->MakeStatic();
	}

	if (strcmp(App->scene->GetCurrentScene(), levelNames.l1Initial.c_str()) == 0) //Initial screen
		runStats.runTime = 0.f;

	//Start Dialogs based on scene & Advance Story
	if (dialogManager != nullptr)
	{
		if (strcmp(App->scene->GetCurrentScene(),levelNames.l1Boss.c_str()) == 0 )
		{
			if(!storyDialogState.defeatedIG11FirstTime)
				dialogManager->StartDialog("1st Conversation IG-11");
			else
				dialogManager->StartDialog("Pool Conversation IG-11");
			return;
		}

		if (strcmp(App->scene->GetCurrentScene(), levelNames.ruinsBoss.c_str()) == 0)
		{
			if (!storyDialogState.defeatedIG12FirstTime)
				dialogManager->StartDialog("1st Conversation with IG - 12");
			else
				dialogManager->StartDialog("Pool Conversation IG-12");
			return;
		}

		if (strcmp(App->scene->GetCurrentScene(), levelNames.hub.c_str()) == 0)
		{
			if (!storyDialogState.firstTimeHub)
			{
				storyDialogState.firstTimeHub = true;
				dialogManager->StartDialog("1st Conversation Cantine");
			}
			else
				dialogManager->StartDialog("Pool Conversation Cantine Death");

			//Add attempt
			runStats.attempt++;

			return;
		}
		
		//dialogManager->StartDialog("GroguHello");
	}	

	clearedRoomAudio = new C_AudioSource(gameObject);

	if (clearedRoomAudio != nullptr)
	{
		clearedRoomAudio->SetEvent("room_cleared");
	}

	//Secondary weapon
	if (playerScript != nullptr)
	{
		WeaponType weaponType = playerScript->GetSecondaryWeapon()->type;

		switch (weaponType)
		{
		case WeaponType::MINIGUN:
			runStats.weaponUsed = "Minigun";
			break;
		case WeaponType::SHOTGUN:
			runStats.weaponUsed = "Shotgun";
			break;
		case WeaponType::SNIPER:
			runStats.weaponUsed = "Sniper";
			break;
		}
	}

	//kills stats
	runStats.runKills += enemies.size();
}

void GameManager::Update()
{
	if(!instantiatedSandstorm) //Instantiate sandstorm
		if (strcmp(App->scene->GetCurrentScene(), levelNames.hub.c_str()) != 0 && strcmp(App->scene->GetCurrentScene(), levelNames.loseScene.c_str()) != 0 
			&& strcmp(App->scene->GetCurrentScene(), "Credits") != 0 && strcmp(App->scene->GetCurrentScene(), "MainMenu") != 0
			&& strcmp(App->scene->GetCurrentScene(), levelNames.winScene.c_str()) != 0)
		{
			App->scene->InstantiatePrefab(mistPlane1.uid, gameObject, mistPlane1Position, Quat::identity);
			App->scene->InstantiatePrefab(mistPlane2.uid, gameObject, mistPlane2Position, Quat::identity);
			instantiatedSandstorm = true;
		}

	//if(groguGameObject != nullptr)
		//LOG("Grogu Pos: x %d, y %d, z %d", groguGameObject->transform->GetLocalPosition().x, groguGameObject->transform->GetLocalPosition().y, groguGameObject->transform->GetLocalPosition().z);

	// --- Handle Camera cutscene
	if (dialogManager != nullptr)
	{
		if (dialogManager->GetDialogState() != DialogState::NO_DIALOG && cameraScript->destinationPoints != nullptr)
			doCameraCutscene = true;
		else
			doCameraCutscene = false;
	}	
		
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

	if(!paused)
		runStats.runTime += MC_Time::Game::GetDT();

	UpdateLeaveBoss();

	//S'ha de fer alguna manera de avisar l'scene que volem canviar de scene pero no fer-ho imediatament ??? -> si (wtf is this (Pau))
	//--
}

void GameManager::CleanUp()
{
	RELEASE(clearedRoomAudio);
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
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.hub + ".json").c_str()))
				//AddFixedRoom("InitialL1", 1, 1);
				level1.insert(level1.begin(), (std::string(ASSETS_SCENES_PATH) + levelNames.hub + ".json"));
			//LEVEL2
			//if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "InitialL2.json").c_str()))
			//	AddFixedRoom("InitialL2", 2, 1);
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.l1Initial + ".json").c_str()))
				//AddFixedRoom("InitialL1", 1, 1);
				level1.insert(level1.begin() + 1, (std::string(ASSETS_SCENES_PATH) + levelNames.l1Initial + ".json"));
			//LEVEL2
			//if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "InitialL2.json").c_str()))
			//	AddFixedRoom("InitialL2", 2, 1);

			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.l1Shop + ".json").c_str()))
				//AddFixedRoom("ShopL1", 1, 4);
				level1.insert(level1.begin() + 6, (std::string(ASSETS_SCENES_PATH) + levelNames.l1Shop + ".json"));
			//LEVEL2
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.ruinsShop + ".json").c_str()))
				level1Ruins.insert(level1Ruins.begin() + 3, (std::string(ASSETS_SCENES_PATH) + levelNames.ruinsShop + ".json"));

			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.l1Boss + ".json").c_str()))
				//AddFixedRoom("BossL1", 1, 10);
				level1.push_back((std::string(ASSETS_SCENES_PATH) + levelNames.l1Boss + ".json"));
			//Second Shop
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.ruinsShop + ".json").c_str()))
				level1Ruins.push_back((std::string(ASSETS_SCENES_PATH) + levelNames.ruinsShop + ".json"));
			//LEVEL2
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.ruinsBoss + ".json").c_str()))
				//AddFixedRoom("BossL2", 2, 10);
				level1Ruins.push_back((std::string(ASSETS_SCENES_PATH) + levelNames.ruinsBoss + ".json"));
			//LEVEL2
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.winScene + ".json").c_str()))
				level1Ruins.push_back((std::string(ASSETS_SCENES_PATH) + levelNames.winScene + ".json"));

			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "Credits.json").c_str()))
				level1Ruins.push_back((std::string(ASSETS_SCENES_PATH) + "Credits.json"));
		}
		else
		{
			//Remove fixed before randomizing
			for (int i = 0; i < level1.size(); ++i)
			{
				if (strstr(level1[i].c_str(), (levelNames.hub + ".json").c_str()) || strstr(level1[i].c_str(), (levelNames.l1Initial + ".json").c_str()) || strstr(level1[i].c_str(), (levelNames.l1Shop + ".json").c_str()) || strstr(level1[i].c_str(), (levelNames.l1Boss + ".json").c_str()))
				{
					level1.erase(level1.begin() + i);
					--i;
				}
			}
			for (int i = 0; i < level1Ruins.size(); ++i)
			{
				if (strstr(level1Ruins[i].c_str(), (levelNames.ruinsShop + ".json").c_str()) || strstr(level1Ruins[i].c_str(), (levelNames.ruinsBoss + ".json").c_str()) || strstr(level1Ruins[i].c_str(), (levelNames.winScene + ".json").c_str()) || strstr(level1Ruins[i].c_str(), "Credits.json"))
				{
					level1Ruins.erase(level1Ruins.begin() + i);
					--i;
				}
			}

			//randomize levels again
			GenerateLevel();

			//add fixed again
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.hub + ".json").c_str()))
				level1.insert(level1.begin(), (std::string(ASSETS_SCENES_PATH) + levelNames.hub + ".json"));
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.l1Initial + ".json").c_str()))
				level1.insert(level1.begin() + 1, (std::string(ASSETS_SCENES_PATH) + levelNames.l1Initial + ".json"));
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.l1Shop + ".json").c_str()))
				level1.insert(level1.begin() + 6, (std::string(ASSETS_SCENES_PATH) + levelNames.l1Shop + ".json"));
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.l1Boss + ".json").c_str()))
				level1.push_back((std::string(ASSETS_SCENES_PATH) + levelNames.l1Boss + ".json"));

			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.ruinsShop + ".json").c_str()))
				level1Ruins.insert(level1Ruins.begin() + 3, (std::string(ASSETS_SCENES_PATH) + levelNames.ruinsShop + ".json"));
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.ruinsShop + ".json").c_str()))
				level1Ruins.push_back((std::string(ASSETS_SCENES_PATH) + levelNames.ruinsShop + ".json"));
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.ruinsBoss + ".json").c_str()))
				level1Ruins.push_back((std::string(ASSETS_SCENES_PATH) + levelNames.ruinsBoss + ".json"));
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + levelNames.winScene + ".json").c_str()))
				level1Ruins.push_back((std::string(ASSETS_SCENES_PATH) + levelNames.winScene + ".json"));
			if (App->fileSystem->Exists((std::string(ASSETS_SCENES_PATH) + "Credits.json").c_str()))
				level1Ruins.push_back((std::string(ASSETS_SCENES_PATH) + "Credits.json"));
		}

		SaveManagerState();
	}
}

void GameManager::SaveArmorerItemLvl(ParsonNode& node)
{
	ParsonNode items = node.SetNode("ArmorerItemLvl");
	items.SetInteger("armorLvl", armorLvl);
	items.SetInteger("bootsLvl", bootsLvl);
	items.SetInteger("ticketLvl", ticketLvl);
	items.SetInteger("bottleLvl", bottleLvl);
}

void GameManager::LoadArmorerItemLvl(ParsonNode& node)
{
	ParsonNode items = node.GetNode("ArmorerItemLvl");
	if (items.NodeIsValid()) 
	{
		armorLvl = items.GetInteger("armorLvl");
		bootsLvl = items.GetInteger("bootsLvl");
		ticketLvl = items.GetInteger("ticketLvl");
		bottleLvl = items.GetInteger("bottleLvl");
	}
}

void GameManager::GenerateLevel()
{
	if (l1Easy >= 0 && l1Easy < level1.size() && l1Intermediate > l1Easy && l1Intermediate < level1.size()) 
	{
		for (int i = 0; i < l1Easy; ++i)
		{
			if (Random::LCG::GetBoundedRandomUint(0, 1) == 1) 
			{
				std::string tmp = level1[i];
				level1[i] = level1[i + 1];
				level1[i + 1] = tmp;
			}		
		}
		for (int i = l1Easy + 1; i < l1Intermediate; ++i)
		{
			if (Random::LCG::GetBoundedRandomUint(0, 1) == 1)
			{
				std::string tmp = level1[i];
				level1[i] = level1[i + 1];
				level1[i + 1] = tmp;
			}
		}
		for (int i = l1Intermediate + 1; i < level1.size()-1; ++i)
		{
			if (Random::LCG::GetBoundedRandomUint(0, 1) == 1)
			{
				std::string tmp = level1[i];
				level1[i] = level1[i + 1];
				level1[i + 1] = tmp;
			}
		}
	}
	else
	{
		for (int i = 0; i < level1.size() - 1; ++i)
		{
			if (Random::LCG::GetBoundedRandomUint(0, 1) == 1)
			{
				std::string tmp = level1[i];
				level1[i] = level1[i + 1];
				level1[i + 1] = tmp;
			}
		}
	}
	
	if (RuinsEasy >= 0 && RuinsEasy < level1Ruins.size() && RuinsIntermediate > RuinsEasy && RuinsIntermediate < level1Ruins.size()) 
	{
		for (int i = 0; i < RuinsEasy; ++i)
		{
			if (Random::LCG::GetBoundedRandomUint(0, 1) == 1)
			{
				std::string tmp = level1Ruins[i];
				level1Ruins[i] = level1Ruins[i + 1];
				level1Ruins[i + 1] = tmp;
			}
		}
		for (int i = RuinsEasy + 1; i < RuinsIntermediate; ++i)
		{
			if (Random::LCG::GetBoundedRandomUint(0, 1) == 1)
			{
				std::string tmp = level1Ruins[i];
				level1Ruins[i] = level1Ruins[i + 1];
				level1Ruins[i + 1] = tmp;
			}
		}
		for (int i = RuinsIntermediate + 1; i < level1Ruins.size() - 1; ++i)
		{
			if (Random::LCG::GetBoundedRandomUint(0, 1) == 1)
			{
				std::string tmp = level1Ruins[i];
				level1Ruins[i] = level1Ruins[i + 1];
				level1Ruins[i + 1] = tmp;
			}
		}
	}
	else
	{
		for (int i = 0; i < level1Ruins.size() - 1; ++i)
		{
			if (Random::LCG::GetBoundedRandomUint(0, 1))
			{
				std::string tmp = level1Ruins[i];
				level1Ruins[i] = level1Ruins[i + 1];
				level1Ruins[i + 1] = tmp;
			}
		}
	}
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
					if (roomNum == 0 && playerScript != nullptr) // this ensures mando starts the run with full health... it's not THAT bad
						playerScript->GiveHeal(999999.0f);
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
		cameraShake = jsonState.GetBool("cameraShake");
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

		//itemsArmorer
		LoadArmorerItemLvl(jsonState);

		//Story
		storyDialogState.Load(&jsonState);

		runStats.Load(&jsonState);

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

void GameManager::Pause()
{
	paused = true;
	std::vector<GameObject*>* objects = App->scene->GetGameObjects();
	for (auto go = objects->begin(); go != objects->end(); ++go)
	{
		for (uint i = 0; i < (*go)->components.size(); ++i)
			if ((*go)->components[i]->GetType() == ComponentType::SCRIPT)
			{
				C_Script* com = (C_Script*)(*go)->components[i];
				if (com != nullptr)
				{
					void* script = com->GetScriptData();
					if (script != nullptr)
						((Script*)script)->OnPause();
				}
			}
	}
}

void GameManager::Resume()
{
	paused = false;
	std::vector<GameObject*>* objects = App->scene->GetGameObjects();
	for (auto go = objects->begin(); go != objects->end(); ++go)
	{
		for (uint i = 0; i < (*go)->components.size(); ++i)
			if ((*go)->components[i]->GetType() == ComponentType::SCRIPT)
			{
				C_Script* com = (C_Script*)(*go)->components[i];
				if (com != nullptr)
				{
					void* script = com->GetScriptData();
					if (script != nullptr)
						((Script*)script)->OnResume();
				}
			}
	}
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
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KeyState::KEY_REPEAT)																// ATTENTION: Could collide with other LCTRL uses.
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
	jsonState.SetBool("cameraShake", cameraShake);
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

	//itemsArmorer
	SaveArmorerItemLvl(jsonState);

	storyDialogState.Save(&jsonState);

	if (strcmp(App->scene->GetCurrentScene(), levelNames.hub.c_str()) == 0) //Reset run time to 0 when starting run
		runStats.runTime = 0;

	runStats.runKills += enemies.size();

	runStats.Save(&jsonState);

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

			int alive = 0;
			for (auto enemy = enemies.begin(); enemy != enemies.end(); ++enemy)
			{
				if ((*enemy).second->health > 0)
					++alive;
				else if (!(*enemy).first)
				{
					(*enemy).first = true;
					lastEnemyDead = (*enemy).second;
				}
			}

			if (alive > 0)
				return;

			gate->Unlock();

			if(clearedRoomAudio != nullptr)
				clearedRoomAudio->PlayFx("room_cleared");

			DropChest();
		}
}

void GameManager::DropChest()
{
	awaitingChestDrop = false;
	uint num = Random::LCG::GetBoundedRandomUint(0, 100);
	if (num <= chestSpawnChance && chestPrefab.uid != NULL)
	{
		GameObject* chest = App->resourceManager->LoadPrefab(chestPrefab.uid, App->scene->GetSceneRoot());
		if (chest != nullptr && lastEnemyDead != nullptr)
		{
			float3 position = lastEnemyDead->transform->GetWorldPosition();
			position.y += 10.0f;
			chest->transform->SetWorldPosition(position);

			float2 playerPosition, chestPosition;
			playerPosition.x = playerGameObject->transform->GetWorldPosition().x;
			playerPosition.y = playerGameObject->transform->GetWorldPosition().z;
			chestPosition.x = chest->transform->GetWorldPosition().x;
			chestPosition.y = chest->transform->GetWorldPosition().z;

			float2 direction = playerPosition - chestPosition;
			if (!direction.IsZero())
				direction.Normalize();
			float rad = direction.AimedAngle();
			chest->transform->SetLocalRotation(float3(DegToRad(Random::LCG::GetBoundedRandomFloat(0.0f, 10.0f)), -rad, DegToRad(Random::LCG::GetBoundedRandomFloat(0.0f, 10.0f))));

			chest->GetComponent<C_RigidBody>()->TransformMovesRigidBody(false);
		}
		droppedChest = true;
	}
}

void GameManager::HandleBackgroundMusic()
{
	if (App->scene->GetCurrentScene() == levelNames.hub)
	{
		App->audio->aSourceBackgroundMusic->StopFx(App->audio->aSourceBackgroundMusic->GetEventId());
		App->audio->aSourceBackgroundMusic->SetEvent("canteen_music", true);
	}
	else if ((App->scene->GetCurrentScene() == levelNames.l1Boss) || (App->scene->GetCurrentScene() == levelNames.ruinsBoss))
	{
		App->audio->aSourceBackgroundMusic->StopFx(App->audio->aSourceBackgroundMusic->GetEventId());
		App->audio->aSourceBackgroundMusic->SetEvent("boss_music", true);
	}
	else if (App->scene->GetCurrentScene() == mainMenuScene)
	{
		App->audio->aSourceBackgroundMusic->StopFx(App->audio->aSourceBackgroundMusic->GetEventId());
		App->audio->aSourceBackgroundMusic->SetEvent("menu_music", true);
	}
	else if ((App->scene->GetCurrentScene() == levelNames.winScene) || (App->scene->GetCurrentScene() == levelNames.loseScene) || strcmp(App->scene->GetCurrentScene(),"Credits")==0)
	{
		if (App->audio->aSourceBackgroundMusic->GetEventName() != "credit_music")
		{
			App->audio->aSourceBackgroundMusic->StopFx(App->audio->aSourceBackgroundMusic->GetEventId());
			App->audio->aSourceBackgroundMusic->SetEvent("credit_music", true);
		}
	}
	else
	{
		if (App->audio->aSourceBackgroundMusic->GetEventName() != "rooms_music")
		{
			App->audio->aSourceBackgroundMusic->StopFx(App->audio->aSourceBackgroundMusic->GetEventId());
			App->audio->aSourceBackgroundMusic->SetEvent("rooms_music", true);
		}

	}
}

void GameManager::PickedItemUp()
{
	//start timer to go to next scene
	pickedItemUp = true;
	LOG("Picked Item Up");
}

void GameManager::UpdateLeaveBoss()
{
	if (killedBoss && !tooBad && !awaitingChestDrop)
		if (droppedChest)
		{
			if (pickedItemUp)
				wantToLeaveBoss = true;
		}
		else
			wantToLeaveBoss = true;

	if(dialogManager != nullptr)
		if (wantToLeaveBoss &&  dialogManager->GetDialogState() == DialogState::NO_DIALOG)
		{
			leaveBossTimer += MC_Time::Game::GetDT();

			if (leaveBossTimer >= leaveBossDelay)
			{
				GoNextRoom();
				wantToLeaveBoss = false;
				tooBad = true;
			}
		}
}

void GameManager::LoadItemPool(std::vector<ItemData*>& pool, std::string path)
{
	// Clear the vector if it has data
	while (pool.size())
	{
		delete* pool.begin();
		pool.erase(pool.begin());
	}
	//Load Json state
	char* itemBuffer = nullptr;
	App->fileSystem->Load(path.c_str(), &itemBuffer);
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
		int minimum = itemNode.GetInteger("Min");
		int maximum = itemNode.GetInteger("Max");
		float power = itemNode.GetNumber("Power");
		float duration = itemNode.GetNumber("Duration");
		float chance = itemNode.GetInteger("Chance");
		std::string texturePath = itemNode.GetString("Texture Path");
		pool.emplace_back(new ItemData(name, description, price, rarity, power, duration, chance, minimum, maximum, texturePath));
	}
}

void GameManager::KilledIG11(int bossNum)
{
	if (bossNum == 0)
	{
		LOG("Killed IG11");
		if (!storyDialogState.defeatedIG11FirstTime)
		{
			storyDialogState.defeatedIG11FirstTime = true;
			groguGameObject = App->scene->InstantiatePrefab(groguPrefab.uid, App->scene->GetSceneRoot(), float3::zero, Quat::identity);
			dialogManager->StartDialog("1st Conversation Grogu");
		}
	}
	killedBoss = true;
}

void GameManager::TalkedToArmorer()
{
	LOG("Talked to armorer");
	if (!storyDialogState.talkedToArmorer)
	{
		dialogManager->StartDialog("1st Conversation Armorer");
		storyDialogState.talkedToArmorer = true;
	}
	else
	{
		dialogManager->StartDialog("Pool Conversation Armorer");
	}
}

void GameManager::BoughtFromArmorer()
{
	LOG("Bought from armorer");
	dialogManager->StartDialog("Pool Conversation Armorer Bought");
}

void GameManager::SetUpWinScreen()
{
	GameObject* tmp = App->scene->GetGameObjectByName("AttemptText");
	if(tmp != nullptr)
		tmp->GetComponent<C_UI_Text>()->SetText(std::to_string(runStats.attempt).c_str());

	tmp = App->scene->GetGameObjectByName("KillsText");
	if (tmp != nullptr)
		tmp->GetComponent<C_UI_Text>()->SetText(std::to_string(runStats.runKills).c_str());

	tmp = App->scene->GetGameObjectByName("TimeText");
	if (tmp != nullptr)
	{
		std::string timeString = "";

		int hours, minutes, seconds = 0;
		int time = runStats.runTime;
		LOG("Total time in seconds: %d",time);
		hours = time / 3600;
		LOG("Total hours: %d", hours);
		time = time % 3600;
		LOG("Time after hours: %d", time);

		minutes = time / 60;
		LOG("Total minutes: %d", minutes);
		
		time = time % 60;
		LOG("Time after minutes: %d", time);

		seconds = time;
		LOG("Total seconds: %d", seconds);

		timeString = std::to_string(hours) + ":";
		timeString += std::to_string(minutes) + ":";
		
		if(seconds > 0)
			timeString += std::to_string(seconds);
		else
			timeString += "00";

		tmp->GetComponent<C_UI_Text>()->SetText(timeString.c_str());
	}
		

	tmp = App->scene->GetGameObjectByName("WeaponText");
	if (tmp != nullptr)
		tmp->GetComponent<C_UI_Text>()->SetText(runStats.weaponUsed.c_str());


}

void GameManager::ResetArmorerItemsLvl()
{
	armorLvl = 0;
	bootsLvl = 0;
	ticketLvl = 0;
	bottleLvl = 0;

	//Load Json state
	char* buffer = nullptr;
	App->fileSystem->Load(saveFileName, &buffer);
	ParsonNode jsonState(buffer);
	//release Json File
	CoreCrossDllHelpers::CoreReleaseBuffer(&buffer); buffer = nullptr;
	SaveArmorerItemLvl(jsonState);

	jsonState.SerializeToFile(saveFileName, &buffer);
	CoreCrossDllHelpers::CoreReleaseBuffer(&buffer);
}

GameManager* CreateGameManager() {
	GameManager* script = new GameManager();

	INSPECTOR_CHECKBOX_BOOL(script->enabled);
	INSPECTOR_STRING(script->mainMenuScene);
	INSPECTOR_STRING(script->SpawnPointName);
	INSPECTOR_STRING(script->loseSceneName);
	INSPECTOR_STRING(script->cameraName);
	INSPECTOR_INPUT_INT(script->l1Easy);
	INSPECTOR_INPUT_INT(script->l1Intermediate);
	INSPECTOR_VECTOR_STRING(script->level1);
	INSPECTOR_INPUT_INT(script->RuinsEasy);
	INSPECTOR_INPUT_INT(script->RuinsIntermediate);
	INSPECTOR_VECTOR_STRING(script->level1Ruins);
	INSPECTOR_PREFAB(script->playerPrefab);
	INSPECTOR_PREFAB(script->groguPrefab);
	INSPECTOR_PREFAB(script->chestPrefab);
	INSPECTOR_DRAGABLE_INT(script->chestSpawnChance);

	INSPECTOR_PREFAB(script->mistPlane1);
	INSPECTOR_PREFAB(script->mistPlane2);
	INSPECTOR_SLIDER_FLOAT3(script->mistPlane1Position, -1000.f, 1000.f);
	INSPECTOR_SLIDER_FLOAT3(script->mistPlane2Position, -1000.f, 1000.f);

	return script;
}

void StoryDialogData::Save(ParsonNode *node)
{
	ParsonNode storyNode = node->SetNode("StoryDialog");
	storyNode.SetBool("visitedHUB", visitedHUB);
	storyNode.SetBool("defeatedIG11FirstTime", defeatedIG11FirstTime);
	storyNode.SetBool("defeatedIG12FirstTime", defeatedIG12FirstTime);
	storyNode.SetBool("talkedToArmorer", talkedToArmorer);
	storyNode.SetBool("firstTimeHub", firstTimeHub);
	storyNode.SetBool("talkedToGrogu", talkedToGrogu);
}

void StoryDialogData::Load(ParsonNode *node)
{
	ParsonNode storyNode = node->GetNode("StoryDialog");
	visitedHUB = storyNode.GetBool("visitedHUB");
	defeatedIG11FirstTime = storyNode.GetBool("defeatedIG11FirstTime");
	defeatedIG12FirstTime = storyNode.GetBool("defeatedIG12FirstTime");
	talkedToArmorer = storyNode.GetBool("talkedToArmorer");
	firstTimeHub = storyNode.GetBool("firstTimeHub");
	talkedToGrogu = storyNode.GetBool("talkedToGrogu");
}

void RunStats::Save(ParsonNode* node)
{
	ParsonNode runStateNode = node->SetNode("RunStats");
	runStateNode.SetInteger("attempt", attempt);
	runStateNode.SetInteger("runKills", runKills);
	runStateNode.SetNumber("runTime", runTime);
	runStateNode.SetNumber("runPrecision", runPrecision);
	runStateNode.SetString("weaponUsed", weaponUsed.c_str());
}

void RunStats::Load(ParsonNode* node)
{
	ParsonNode runStateNode = node->GetNode("RunStats");
	attempt = runStateNode.GetInteger("attempt");
	runKills = runStateNode.GetInteger("runKills");
	runTime = runStateNode.GetNumber("runTime");
	runPrecision = runStateNode.GetNumber("runPrecision");
	weaponUsed = runStateNode.GetString("weaponUsed");
}

void RunStats::ResetRun()
{
	runKills = 0;
	runTime = 0.f;
	runPrecision = 0;
}
