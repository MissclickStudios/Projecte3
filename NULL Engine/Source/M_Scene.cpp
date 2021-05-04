#include "Profiler.h"
#include "MathGeoTransform.h"

#include "JSONParser.h"
#include "Random.h"

#include "VariableTypedefs.h"

#include "Application.h"														// ATTENTION: Globals.h already included in Module.h
#include "M_Window.h"
#include "M_Camera3D.h"
#include "M_Renderer3D.h"
#include "M_Input.h"
#include "M_FileSystem.h"
#include "M_Editor.h"
#include "M_ResourceManager.h"
#include "M_UISystem.h"

#include "ResourceBase.h"
#include "Resource.h"
#include "R_Model.h"
#include "R_Mesh.h"
#include "R_Texture.h"
#include "R_Scene.h"

#include "GameObject.h"
#include "Component.h"
#include "C_Transform.h"
#include "C_Mesh.h"
#include "C_Material.h"
#include "C_Camera.h"
#include "C_Animator.h"
#include "C_Light.h"
#include "C_UI_Button.h"
#include "C_AudioSource.h"
#include "C_AudioListener.h"
#include "C_UI_Text.h"


#include "Primitive.h"

#include "M_Scene.h"
#include "M_ScriptManager.h"

#include "MemoryManager.h"

M_Scene::M_Scene(bool isActive) : Module("SceneManager", isActive),
masterRoot				(nullptr),
sceneRoot				(nullptr),
animationRoot			(nullptr),
selectedGameObject		(nullptr),
cullingCamera			(nullptr)
{
	CreateMasterRoot();
	CreateSceneRoot("MainScene");

	selectedGameObject = sceneRoot;
}

M_Scene::~M_Scene()
{
	masterRoot->CleanUp();
	RELEASE(masterRoot);
}

bool M_Scene::Init(ParsonNode& config)
{
	currentScene = config.GetString("currentScene");
	return true;
}

// Load assets
bool M_Scene::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;
	App->camera->LookAt(float3::zero);

	if (sceneRoot == nullptr)
	{
		sceneRoot = CreateGameObject("Main Scene");

		selectedGameObject = sceneRoot;
	}

	CreateSceneCamera("SceneCamera");
	

	if(App->gameState == GameState::PLAY)
		LoadScene("Assets/Scenes/MainMenu.json");
	else
	{
		std::string s = ASSETS_SCENES_PATH + currentScene + JSON_EXTENSION;
		LoadScene(s.c_str());
		//LoadScene("Assets/Scenes/MainMenu.json");
	}

	//LoadScene("Assets/Scenes/UITestScene.json");
	//SaveScene("SceneAutosave");																			// Autosave just right after loading the scene.

	/*C_AudioListener* listener = new C_AudioListener(masterRoot);

	music = new C_AudioSource(masterRoot);

	music->SetEvent("background", 3650723969);

	music->PlayFx(music->GetEventId());*/

	return ret;
}

// Update
UpdateStatus M_Scene::Update(float dt)
{
	OPTICK_CATEGORY("M_Scene Update", Optick::Category::Module)

	HandleCopyGO();

	// --- Sort GameObjects by Z-Buffer value
	//UpdateSceneFromRoot(sceneRoot);
	
	for (uint i = 0; i < gameObjects.size(); ++i)														// First Pass to Delete or Update the GOs' state.
	{	
		if (gameObjects[i]->toDelete)
		{
			DeleteGameObject(gameObjects[i], i);
			continue;
		}

		if (gameObjects[i]->IsActive())
		{
			gameObjects[i]->Update();
		}
	}

	RefreshSceneTransforms();																			// Second pass to make sure that all GOs' World Transform is updated.

	std::vector<MeshRenderer>		meshRenderers;
	std::vector<CuboidRenderer>		cuboidRenderers;
	std::vector<SkeletonRenderer>	skeletonRenderers;
	for (uint i = 0; i < gameObjects.size(); ++i)														// Third pass to get the renderers with the fully updated GOs.
	{
		if (gameObjects[i]->IsActive())
		{
			if (GameObjectIsInsideCullingCamera(gameObjects[i]) || gameObjects[i] == cullingCamera->GetOwner())
			{
				gameObjects[i]->GetRenderers(meshRenderers, cuboidRenderers, skeletonRenderers);
			}
		}
	}

	App->renderer->AddRenderersBatch(meshRenderers, cuboidRenderers, skeletonRenderers);
	
	meshRenderers.clear();
	cuboidRenderers.clear();
	skeletonRenderers.clear();
	
	// --- M_SCENE SHORTCUTS
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KeyState::KEY_REPEAT)
	{
		if (App->gameState != GameState::PLAY)
		{
			if (App->input->GetKey(SDL_SCANCODE_F5) == KeyState::KEY_DOWN)
			{
				App->SaveConfiguration("Resources/Engine/Configuration/configuration.JSON");
			}
			if (App->input->GetKey(SDL_SCANCODE_F6) == KeyState::KEY_DOWN)
			{
				App->LoadConfiguration("Resources/Engine/Configuration/configuration.JSON");
			}
		}
	}
	
	ShowFPS();

	return UpdateStatus::CONTINUE;
}

UpdateStatus M_Scene::PostUpdate(float dt)
{	
	OPTICK_CATEGORY("M_Scene PostUpdate", Optick::Category::Module)

	if (nextScene)
	{
		LoadScene(nextSceneName.c_str());
		nextScene = false;
	}

	return UpdateStatus::CONTINUE;
}

// Load assets
bool M_Scene::CleanUp()
{
	LOG("Unloading Intro scene");
	
	//App->renderer->ClearRenderers();

	App->renderer->defaultSkyBox.CleanUp();

	for (auto object = gameObjects.begin(); object != gameObjects.end(); ++object)
	{
		(*object)->CleanUp();
		RELEASE((*object));
	}
	for (auto item = models.begin(); item != models.end(); ++item)
	{
		App->resourceManager->FreeResource(item->second.first);
	}

	gameObjects.clear();
	models.clear();

	sceneRoot				= nullptr;
	animationRoot			= nullptr;
	cullingCamera			= nullptr;
	selectedGameObject		= nullptr;

	return true;
}

bool M_Scene::SaveConfiguration(ParsonNode& root) const
{
	bool ret = true;

	root.SetString("currentScene", currentScene.c_str());

	return ret;
}

bool M_Scene::LoadConfiguration(ParsonNode& root)
{
	bool ret = true;

	//LoadScene();

	return ret;
}

// -------------- SCENE METHODS --------------
bool M_Scene::SaveScene(const char* sceneName) const
{
	bool ret = true;

	ParsonNode rootNode		= ParsonNode();

	ParsonArray modelArray = rootNode.SetArray("Models In Scene");
	for (auto item = models.begin(); item != models.end(); ++item)
	{
		ParsonNode modelNode = modelArray.SetNode("ModelInScene");
		modelNode.SetNumber("GameObjectUID", (double)item->first);
		modelNode.SetNumber("ModelResourceUID", (double)item->second.first);
		modelNode.SetString("ModelAssetsPath", item->second.second.c_str());
	}

	ParsonArray objectsArray	= rootNode.SetArray("Game Objects");
	for (auto object = gameObjects.begin(); object != gameObjects.end(); ++object)
	{
		ParsonNode arrayNode = objectsArray.SetNode((*object)->GetName());
		(*object)->SaveState(arrayNode);
	}

	if (!strcmp(sceneName,AUTOSAVE_FILE_NAME))
		rootNode.SetString("Autosaved Sene Name", currentScene.c_str());

	char* buffer		= nullptr;
	std::string name	= (sceneName != nullptr) ? sceneName : sceneRoot->GetName();
	std::string path	= ASSETS_SCENES_PATH + name + JSON_EXTENSION;
	uint written = rootNode.SerializeToFile(path.c_str(), &buffer);
	if (written > 0)
	{
		LOG("[SCENE] Scene: Successfully saved the current scene! Path: %s", path.c_str());
	}
	else
	{
		LOG("[ERROR] Scene: Could not save the current scene! Error: FileSystem could not write any data!");
	}

	R_Scene* rScene	= (R_Scene*)App->resourceManager->CreateResource(ResourceType::SCENE, path.c_str());			// TMP until R_Scene is fully implemented.
	//App->resourceManager->SaveResourceToLibrary(r_scene);

	std::string libraryPath	= SCENES_PATH + std::to_string(rScene->GetUID()) + SCENES_EXTENSION;
	written = App->fileSystem->Save(libraryPath.c_str(), buffer, written);
	if (written > 0)
	{
		LOG("[SCENE] Scene: Successfully saved the current scene in Library! Path: %s", libraryPath.c_str());
	}
	else
	{
		LOG("[ERROR] Scene: Could not save the current scene in Library! Error: FileSystem could not write any data!");
	}

	App->resourceManager->DeallocateResource(rScene);

	RELEASE_ARRAY(buffer);

	return ret;
}

bool M_Scene::LoadScene(const char* path)
{
	bool ret = true;

	std::string sceneName;

	App->fileSystem->SplitFilePath(path, nullptr, &sceneName);

	if(sceneName != AUTOSAVE_FILE_NAME)
		currentScene = sceneName;

	App->camera->SetMasterCameraAsCurrentCamera();

	char* buffer = nullptr;
	uint read = App->fileSystem->Load(path, &buffer);
	if (read == 0)
	{
		LOG("[ERROR] Scene Loading: Could not load %s from Assets! Error: File system could not read the file!", path);
		return false;
	}

	int modTime = App->fileSystem->GetLastModTime(path);
	std::vector<GameObject*> prefabsToDelete;

	if (buffer != nullptr)
	{
		App->renderer->ClearRenderers();
		if (App->gameState == GameState::PLAY)
			App->scriptManager->CleanUpScripts();
		CleanUp();
		App->uiSystem->ClearActiveCanvas();

		//std::vector<GameObject*> parentMaintained;
		//CleanUpCurrentScene(parentMaintained);

		ParsonNode newRoot			= ParsonNode(buffer);
		ParsonArray modelsArray		= newRoot.GetArray("Models In Scene");
		ParsonArray objectsArray	= newRoot.GetArray("Game Objects");
		RELEASE_ARRAY(buffer);

		if (sceneName == AUTOSAVE_FILE_NAME)
			currentScene = newRoot.GetString("Autosaved Sene Name");

		for (uint i = 0; i < modelsArray.size; ++i)
		{
			ParsonNode modelNode = modelsArray.GetNode(i);
			if (!modelNode.NodeIsValid())
			{
				continue;
			}

			uint32 gameObjectUID		= (uint32)modelNode.GetNumber("GameObjectUID");
			uint32 modelResourceUID		= (uint32)modelNode.GetNumber("ModelResourceUID");
			const char* modelAssetsPath = modelNode.GetString("ModelAssetsPath");

			if (App->resourceManager->GetResourceFromLibrary(modelAssetsPath) != nullptr)
			{
				std::pair<uint32, std::string> modelResource = { modelResourceUID, modelAssetsPath };
				models.emplace(gameObjectUID, modelResource);
			}
		}

		std::map<uint32, GameObject*> tmp;

		for (uint i = 0; i < objectsArray.size; ++i)																			// Getting all the GameObjects in the ParsonArray
		{

			ParsonNode objectNode = objectsArray.GetNode(i);
			if (!objectNode.NodeIsValid())
			{
				continue;
			}

			GameObject* gameObject = new GameObject();

			gameObject->LoadState(objectNode);

			if (gameObject->isSceneRoot)
			{
				sceneRoot = gameObject;
				sceneRoot->SetParent(masterRoot);

				/*for (uint i = 0; i < parentMaintained.size(); ++i)
				{
					parentMaintained[i]->SetParent(sceneRoot);
				}

				parentMaintained.clear();*/
			}

			if (gameObject->GetComponent<C_Animator>() != nullptr)
			{
				animationRoot = gameObject;
			}

			C_Camera* cCamera = gameObject->GetComponent<C_Camera>();
			if (cCamera != nullptr)
			{
				if (cCamera->IsCulling())
				{
					cullingCamera = cCamera;
				}
			}

			tmp.emplace(gameObject->GetUID(), gameObject);
		}

		std::map<uint, Prefab> prefabs = App->resourceManager->prefabs;
		

		// Re-Parenting
		std::map<uint32, GameObject*>::iterator item;
		for (item = tmp.begin(); item != tmp.end(); ++item)
		{
			uint parentUid = item->second->GetParentUID();
			if (parentUid == 0)
				continue;

			if (item->second->isPrefab && item->second->prefabID != 0)
			{
				std::map<uint32, Prefab>::iterator prefab = prefabs.find(item->second->prefabID);
				if (prefab != prefabs.end())
				{
					if (prefab->second.updateTime > modTime) //If prefab is older then load prefab/ignore non parent 
					{
						std::map<uint32,GameObject*>::iterator prefabParent = tmp.find(item->second->GetParentUID());
						if (prefabParent != tmp.end())
						{
							if (prefabParent->second->prefabID == prefab->second.uid) //If parent is the same prefab then ignore object
							{
								prefabsToDelete.push_back(item->second);
								continue;
							}
							else //Then it's parent, so load prefab
							{
								App->resourceManager->LoadPrefab(prefab->second.uid, prefabParent->second, item->second);
								prefabsToDelete.push_back(item->second);
								continue;
							}
						}
					}
				}
			}

			std::map<uint32, GameObject*>::iterator parent = tmp.find(parentUid);
			if (parent != tmp.end())
			{
				item->second->SetParent(parent->second);
			}

			item->second->GetComponent<C_Transform>()->Translate(float3::zero);						// Dirty way to refresh the transforms after the import is done. TMP Un-hardcode later.
			//AddGameObjectToVector(item->second);
			gameObjects.push_back(item->second);
		}
		
		tmp.clear();
		App->renderer->ClearRenderers();
	}

	for (auto p = prefabsToDelete.begin(); p != prefabsToDelete.end(); ++p)
	{
		(*p)->CleanUp();
		RELEASE(*p);
	}

	//Resolve script go pointers reassigning
	if (!toAdd.empty()) {
		std::vector< std::pair<uint32, GameObject**>>::const_iterator item = toAdd.cbegin();
		for (; item != toAdd.cend(); ++item) {
			GameObject* found = GetGameObjectByUID((*item).first);
			if (found != nullptr) {
				*(*item).second = found;
			}
			else
				LOG("ScriptGameObject: Game Object with UID %d not found to resolve script pointer", (*item).first);
		}
		toAdd.clear();
	}

	//FIX THIS
	App->renderer->defaultSkyBox.SetUpSkyBoxBuffers();

	LOG("Successfully Loaded Scene: %s", path);

	//if (!CheckSceneLight()) AddSceneLight(App->renderer->GenerateSceneLight(Color(1.0f, 1.0f, 1.0f, 1.0f), Color(0.6, 0.6, 0.6, 0.5), Color(0.6, 0.6, 0.6, 0.5), LightType::DIRECTIONAL));
	if (App->gameState == GameState::PLAY)
		App->scriptManager->InitScripts();

	return ret;
}

bool M_Scene::CleanUpCurrentScene(std::vector<GameObject*>& parentMaintained)						// ATTENTION: Look for a way to erase an item in a loop and keep iter. without problems.
{
	LOG("Unloading { %s } Scene", currentScene.c_str());

	App->renderer->defaultSkyBox.CleanUp();

	for (auto object = gameObjects.begin(); object != gameObjects.end(); ++object)					// ATTENTION: There are a total of 6 loops when only 2 should be needed. Revise later.
	{
		if ((*object)->GetMaintainThroughScenes() && (*object)->parent == sceneRoot)
		{
			parentMaintained.push_back((*object));												// If not done first all parent pointers get corrupted or set to NULL.
		}
	}

	std::map<uint32, std::string> goToDelete;
	std::vector<uint32> modelsToDelete;
	std::vector<GameObject*> why;
	for (auto object = gameObjects.begin(); object != gameObjects.end(); ++object)
	{
		if (!(*object)->GetMaintainThroughScenes())
		{
			goToDelete.emplace((*object)->GetUID(), (*object)->GetName());				// Getting the GameObjects that should be deleted.

			(*object)->CleanUp();
			RELEASE((*object));
		}
	}

	for (auto model = models.begin(); model != models.end(); ++model)
	{
		if (goToDelete.find(model->first) != goToDelete.end())
		{
			App->resourceManager->FreeResource(model->second.first);								// Getting the models that should be deleted.
			modelsToDelete.push_back(model->first);
		}
	}

	for (auto object = goToDelete.begin(); object != goToDelete.end(); ++object)
	{	
		/*(gameObjects.find(object->first) != gameObjects.end())	? gameObjects.erase(object->first) : LOG("[ERROR] Scene: Could not Delete { %s }! Error: GO amiss", object->second.c_str());
		(goNamesMap.find(object->second) != goNamesMap.end())	? goNamesMap.erase(object->second) : LOG("[ERROR] Scene: Could not Delete { %s }! Error: Name amiss", object->second.c_str());*/
	}
	for (auto model = modelsToDelete.begin(); model != modelsToDelete.end(); ++model)
	{
		(models.find((*model)) != models.end()) ? models.erase((*model)) : LOG("[ERROR] Scene: Could not Delete Model { %lu }! Error: Model could not be found in map", (*model));
	}

	for (auto object = gameObjects.cbegin(); object != gameObjects.cend(); ++object)				// Making sure that the remaining GOs (maintain) are not set to delete.
	{
		//(object->second != nullptr) ? object->second->toDelete = false : LOG("[ERROR] Scene: GameObject remaining after Current Scene CleanUp was nullptr!");
	}

	sceneRoot			= nullptr;
	animationRoot		= nullptr;
	cullingCamera		= nullptr;
	selectedGameObject	= nullptr;
	
	return false;
}

void M_Scene::SaveCurrentScene()
{
	SaveScene(currentScene.c_str());
}

bool M_Scene::SaveSceneAs(const char* sceneName)
{
	currentScene = sceneName;
	SaveScene(sceneName);

	return true;
}

bool M_Scene::NewScene()
{
	App->renderer->ClearRenderers();
	CleanUp();

	CreateSceneRoot("MainScene");

	App->renderer->defaultSkyBox.SetUpSkyBoxBuffers();

	currentScene = "New Scene";

	return true;
}

void M_Scene::UpdateSceneFromRoot(GameObject* root)
{	
	if (root->toDelete)
	{
		root->FreeChilds();
		
		DeleteGameObject(root);
	}
	else
	{
		if (root->IsActive())
		{
			root->Update();

			root->GetComponent<C_Transform>()->GetWorldTransform();

			if (GameObjectIsInsideCullingCamera(root) || root == cullingCamera->GetOwner())
			{
				std::vector<MeshRenderer>		meshRenderers;
				std::vector<CuboidRenderer>		cuboidRenderers;
				std::vector<SkeletonRenderer>	skeletonRenderers;
				
				root->GetRenderers(meshRenderers, cuboidRenderers, skeletonRenderers);

				App->renderer->AddRenderersBatch(meshRenderers, cuboidRenderers, skeletonRenderers);

				skeletonRenderers.clear();
				cuboidRenderers.clear();
				meshRenderers.clear();
			}
		}
	}

	for (uint i = 0; i < root->childs.size(); ++i)
	{
		UpdateSceneFromRoot(root->childs[i]);
	}
}

void M_Scene::RefreshSceneTransforms()
{
	sceneRoot->GetComponent<C_Transform>()->RefreshTransformsChain();
}

void M_Scene::LoadResourceIntoScene(Resource* resource)
{
	if (resource == nullptr)
	{
		LOG("[ERROR] Scene: Could not Load Resource Into Scene! Error: Given Resource* was nullptr.");
		return;
	}

	bool success = false;

	switch (resource->GetType())
	{
	case::ResourceType::MODEL:		{ GenerateGameObjectsFromModel((R_Model*)resource); }				break;
	case::ResourceType::TEXTURE:	{ success = ApplyTextureToSelectedGameObject(resource->GetUID()); }	break;
	}
}

GameObject* M_Scene::LoadPrefabIntoScene(ParsonNode* a, GameObject* parent)
{
	GameObject* gameObject = new GameObject();

	gameObject->LoadState(*a);

	parent != nullptr ? gameObject->SetParent(parent) : gameObject->SetParent(App->scene->GetSceneRoot());

	AddGameObjectToVector(gameObject);

	ParsonArray childArray = a->GetArray("Children");

	gameObject->ForceUID(Random::LCG::GetRandomUint());

	for (int i = 0; i < childArray.size; i++)
	{
		App->scene->LoadPrefabObject(gameObject, &childArray.GetNode(i));
	}

	return gameObject;
}

void M_Scene::LoadPrefabObject(GameObject* _gameObject, ParsonNode* node)
{
	GameObject* gameObject = new GameObject();

	gameObject->LoadState(*node);

	(_gameObject != nullptr) ? gameObject->SetParent(_gameObject) : gameObject->SetParent(sceneRoot);
	
	AddGameObjectToVector(gameObject);

	gameObject->ForceUID(Random::LCG::GetRandomUint());

	ParsonArray childArray = node->GetArray("Children");
	for (int i = 0; i < childArray.size; i++)
	{
		LoadPrefabObject(gameObject, &childArray.GetNode(i));
	}
}

GameObject* M_Scene::InstantiatePrefab(uint prefabID, GameObject* parent, float3 position, Quat rotation)
{
	char* buffer = nullptr;
	std::string fileName = ASSETS_PREFABS_PATH + std::to_string(prefabID) + PREFAB_EXTENSION;
	uint f = App->fileSystem->Load(fileName.c_str(), &buffer);
	if (f == 0)
	{
		LOG("Could not load prefab with ID: %d into scene", prefabID);
		return nullptr;
	}

	ParsonNode prefabRoot(buffer);
	RELEASE_ARRAY(buffer);

	GameObject* rootObjectLoaded = App->scene->LoadPrefabIntoScene(&prefabRoot, parent);

	rootObjectLoaded->transform->SetLocalPosition(position);
	rootObjectLoaded->transform->SetLocalRotation(rotation);

	return rootObjectLoaded;
}

GameObject* M_Scene::CreateGameObject(const char* name, GameObject* parent)
{	
	if (gameObjects.empty())
	{
		CreateSceneRoot(name);
		return sceneRoot;
	}

	if (name == nullptr)
		strcpy((char*)name, "ObjectWithoutName");

	GameObject* gameObject = new GameObject(name);

	if (gameObject != nullptr)
	{
		(parent != nullptr) ? gameObject->SetParent(parent) : gameObject->SetParent(sceneRoot);					// Just in case.

		AddGameObjectToVector(gameObject);
	}

	return gameObject;
}

void M_Scene::DeleteGameObject(GameObject* gameObject,  int index)
{
	if (gameObject == nullptr)
	{
		LOG("[ERROR] Scene: Object to delete was nullptr!");
		return;
	}

	if (selectedGameObject == gameObject)
	{
		selectedGameObject = nullptr;
	}
	if (gameObject == animationRoot)
	{
		animationRoot = nullptr;
	}
	
	auto item = models.find(gameObject->GetUID());
	if (item != models.end())
	{
		App->resourceManager->FreeResource(item->second.first);
		models.erase(item);
	}
	
	std::vector<C_Mesh*> cMeshes;
	bool foundMeshes = gameObject->GetComponents<C_Mesh>(cMeshes);
	if (foundMeshes)
	{
		for (uint i = 0; i < cMeshes.size(); ++i)
		{
			App->renderer->DeleteFromMeshRenderers(cMeshes[i]);					// ATTENTION: Really slow! Revise Renderers later.
		}

		cMeshes.clear();
	}

	if (!gameObjects.empty())													// Extra check just to make sure that at least one GameObject remains in the Scene.
	{
		gameObject->CleanUp();													// As it has not been Cleaned Up by its parent, the GameObject needs to call its CleanUp();
		uint32 goUID = gameObject->GetUID();

		if (index != -1)														// If an index was given.
		{
			gameObjects.erase(gameObjects.begin() + index);						// Delete game object at index.
		}
		else
		{
			for (uint i = 0; i < gameObjects.size(); ++i)						// If no index was given.
			{
				if (gameObjects[i] == gameObject)								// Iterate game_objects until a match is found.
				{
					gameObjects.erase(gameObjects.begin() + i);					// Delete the game_object at the current loop index.
					break;
				}
			}
		}

		RELEASE(gameObject);
	}		
}

void M_Scene::AddGameObjectToVector(GameObject* gameObject)
{
	if (gameObject == nullptr)
	{
		LOG("[ERROR] Scene: Could not Add Game Object to Maps! Error: Given GameObject* was nullptr.");
		return;
	}

	gameObjects.push_back(gameObject);
}

void M_Scene::AddGameObjectToScene(GameObject* gameObject, GameObject* parent)
{
	if (gameObject == nullptr)
	{
		LOG("[ERROR] Scene: Could not Add Game Object to Scene! Error: Given GameObject* was nullptr.");
		return;
	}
	
	parent != nullptr ? gameObject->SetParent(sceneRoot) : gameObject->SetParent(parent);
	
	AddGameObjectChildrenToScene(gameObject);
}

void M_Scene::AddGameObjectChildrenToScene(GameObject* gameObject)
{
	if (gameObject == nullptr)
	{
		LOG("[ERROR] Scene: Could not Add Game Object's Children to Scene! Error: Given GameObject* was nullptr.");
		return;
	}
	
	AddGameObjectToVector(gameObject);

	for (auto child = gameObject->childs.begin(); child != gameObject->childs.end(); ++child)
	{
		AddGameObjectChildrenToScene(*child);
	}
}

GameObject* M_Scene::GenerateGameObjectsFromModel(const R_Model* rModel, const float3& scale)
{
	//R_Model* rModel = (R_Model*)App->resourceManager->RequestResource(modelUid);

	if (rModel == nullptr)
	{
		LOG("[ERROR] Scene: Could not generate GameObjects from Model Resource! Error: R_Model* was nullptr.");
		return nullptr;
	}

	GameObject* parentRoot = nullptr;
	std::map<uint32, GameObject*> tmp;

	std::vector<ModelNode> mNodes = rModel->modelNodes;
	for (uint i = 0; i < mNodes.size(); ++i)
	{
		GameObject* gameObject = new GameObject();

		gameObject->ForceUID(mNodes[i].uid);
		gameObject->SetParentUID(mNodes[i].parentUID);
		gameObject->SetName(mNodes[i].name.c_str());
		gameObject->GetComponent<C_Transform>()->ImportTransform(mNodes[i].transform);

		CreateComponentsFromModelNode(mNodes[i], gameObject);

		if (mNodes[i].parentUID == 0)
		{
			parentRoot = gameObject;

			std::pair<uint32, std::string> modelResource = { rModel->GetUID(), rModel->GetAssetsPath() };
			models.emplace(parentRoot->GetUID(), modelResource);
		}

		tmp.emplace(gameObject->GetUID(), gameObject);
	}
	  
	mNodes.clear();

	// Re-Parenting
	std::map<uint32, GameObject*>::iterator item;
	for (item = tmp.begin(); item != tmp.end(); ++item)
	{
		uint parentUid = item->second->GetParentUID();
		if (parentUid == 0)
		{
			item->second->SetParent(sceneRoot);
		}
		else
		{
			std::map<uint32, GameObject*>::iterator parent = tmp.find(parentUid);
			if (parent != tmp.end())
			{
				item->second->SetParent(parent->second);
			}
		}

		item->second->GetComponent<C_Transform>()->Translate(float3::zero);						// Dirty way to refresh the transforms after the import is done. TMP Un-hardcode later.
		AddGameObjectToVector(item->second);
	}

	if (parentRoot != nullptr)
	{
		if (scale.x != 0.0f)																	// Dirty way of detecting that the scale is valid.
		{
			parentRoot->GetComponent<C_Transform>()->SetLocalScale(scale);
		}

		if (!rModel->animations.empty())
		{
			CreateAnimationComponentFromModel(rModel, parentRoot);							// Must be done last as the parent hierarchy needs to be in place.
		}

		//parentRoot = nullptr; //I, David Rami, purposefully commented this for a reason.
	}

	tmp.clear();

	return parentRoot;
}

void M_Scene::CreateComponentsFromModelNode(const ModelNode& modelNode, GameObject* gameObject)
{
	bool validMeshUid			= (modelNode.meshUID != 0)		?	true : false;
	bool validMaterialUid		= (modelNode.materialUID != 0)	?	true : false;
	bool validTextureUid		= (modelNode.textureUID != 0)	?	true : false;
	bool validShaderUid			= (modelNode.shaderUID != 0)	?	true : false;
	
	// Set Mesh
	if (validMeshUid)
	{
		C_Mesh* cMesh = (C_Mesh*)gameObject->CreateComponent(ComponentType::MESH);
		R_Mesh* rMesh = (R_Mesh*)App->resourceManager->RequestResource(modelNode.meshUID);

		if (rMesh == nullptr)
		{
			LOG("[ERROR] Scene: Could not generate the Mesh Resource from the Model Node! Error: R_Mesh* could not be found in resources.");
			gameObject->DeleteComponent(cMesh);
			return;
		}

		cMesh->SetMesh(rMesh);
	}

	// Set Material
	if (validMaterialUid)
	{
		C_Material* cMaterial = (C_Material*)gameObject->CreateComponent(ComponentType::MATERIAL);
		R_Material* rMaterial = (R_Material*)App->resourceManager->RequestResource(modelNode.materialUID);
		
		if (rMaterial == nullptr)
		{
			LOG("[ERROR] Scene: Could not generate the Material Resource from the Model Node! Error: R_Material* could not be found in resources.");
			gameObject->DeleteComponent(cMaterial);
			return;
		}

		cMaterial->SetMaterial(rMaterial);
		

		// Set Texture
		if (validTextureUid)
		{
			R_Texture* r_texture = (R_Texture*)App->resourceManager->RequestResource(modelNode.textureUID);
			if (r_texture == nullptr)
			{
				LOG("[ERROR] Scene: Could not generate the Texture Resource from the Model Node! Error: R_Texture* could not be found in resources.");
				return;
			}


			cMaterial->SetTexture(r_texture);
		}

		if (validShaderUid)
		{
			R_Shader* rShader = (R_Shader*)App->resourceManager->RequestResource(modelNode.shaderUID);
			if (rShader == nullptr)
			{
				LOG("[ERROR] Scene: Could not generate the Shader Resource from the Model Node! Error: R_Shader* could not be found in resources.");
				return;
			}
			cMaterial->SetShader(rShader);
		}
	}
}

void M_Scene::CreateAnimationComponentFromModel(const R_Model* rModel, GameObject* gameObject)
{
	if (rModel == nullptr)
	{
		LOG("[ERROR] Scene: Could not Create Animation Component From Model! Error: Given R_Model* was nullptr.");
		return;
	}
	if (gameObject == nullptr)
	{
		LOG("[ERROR] Scene: Could not Create Animation Component From Model! Error: Given GameObject* was nullptr.");
		return;
	}
	if (rModel->animations.empty())
	{
		LOG("[ERROR] Scene: Could not Create Animation Component From Model! Error: Given R_Model* had no animations.");
		return;
	}

	animationRoot = gameObject;

	C_Animator* cAnimation = (C_Animator*)gameObject->CreateComponent(ComponentType::ANIMATOR);
	std::map<uint32, std::string>::const_iterator item;
	for (item = rModel->animations.cbegin(); item != rModel->animations.cend(); ++item)
	{
		R_Animation* rAnimation = (R_Animation*)App->resourceManager->RequestResource(item->first);
		if (rAnimation != nullptr)
		{
			cAnimation->AddAnimation(rAnimation);
		}
	}
}

std::vector<GameObject*>* M_Scene::GetGameObjects()
{
	return &gameObjects;
}

bool M_Scene::ApplyTextureToSelectedGameObject(const uint32& uid)
{

	if (selectedGameObject == nullptr)
	{
		LOG("[ERROR] Could not add the texture to the selected game object! Error: No game object was being selected.");
		return false;
	}
	if (uid == 0)
	{
		LOG("[ERROR] Could not add the texture to the selected game object! Error: Given Resource UID was 0.");
		return false;
	}

	R_Texture* rTexture = (R_Texture*)App->resourceManager->RequestResource(uid);

	if (rTexture == nullptr)
	{
		LOG("[ERROR] Could not add the texture to the selected game object! Error: R_Texture* was nullptr.");
		return false;
	}
	if (rTexture->GetTextureID() == 0)
	{
		LOG("[ERROR] Could not add the texture to the selected game object! Error: R_Texture* Texture ID was 0.");
		App->resourceManager->DeallocateResource(rTexture);
		return false;
	}

	// --- SETTING THE NEW TEXTURE ---
	C_Material* cMaterial = selectedGameObject->GetComponent<C_Material>();									// GetMaterialComponent() == nullptr if GO does not have a C_Material.
	if (cMaterial == nullptr)
	{
		cMaterial = (C_Material*)selectedGameObject->CreateComponent(ComponentType::MATERIAL);				// Creating a Material Component if none was found in the selected GO.
	}

	cMaterial->SetTexture(rTexture);																			// Setting the Material Component's texture with the newly created one.

	return true;
}

void M_Scene::CreateMasterRoot()
{
	masterRoot = new GameObject("MasterRoot");
	masterRoot->isMasterRoot = true;
}

void M_Scene::DeleteMasterRoot()
{
	if (masterRoot != nullptr)
	{
		masterRoot->CleanUp();
		RELEASE(masterRoot);
	}
}

GameObject* M_Scene::GetMasterRoot() const
{
	return masterRoot;
}

const char* M_Scene::GetCurrentScene() const
{
	return currentScene.c_str();
}

void M_Scene::CreateSceneRoot(const char* sceneName)
{
	if (masterRoot == nullptr)
	{
		CreateMasterRoot();																	// Safety Check.
	}
	
	sceneRoot = new GameObject(sceneName);

	sceneRoot->isSceneRoot = true;

	sceneRoot->SetParent(masterRoot);
	
	//scene_root->parent = master_root;
	//master_root->AddChild(scene_root);

	AddGameObjectToVector(sceneRoot);
}

GameObject* M_Scene::GetSceneRoot() const
{
	return sceneRoot;
}

void M_Scene::SetSceneRoot(GameObject* gameObject)
{
	if (gameObject != sceneRoot)
	{
		sceneRoot = gameObject;
	}
}

void M_Scene::ChangeSceneName(const char* name)
{
	sceneRoot->SetName(name);
}

void M_Scene::CreateSceneCamera(const char* cameraName)
{
	GameObject* sceneCamera = CreateGameObject(cameraName, sceneRoot);
	sceneCamera->CreateComponent(ComponentType::CAMERA);
	sceneCamera->GetComponent<C_Camera>()->SetAspectRatio((float)App->window->GetWidth() / (float)App->window->GetHeight());
	sceneCamera->GetComponent<C_Transform>()->SetLocalPosition(float3(0.0f, 5.0f, 25.0f));
}

C_Camera* M_Scene::GetCullingCamera() const
{
	return cullingCamera;
}

void M_Scene::SetCullingCamera(C_Camera* cullingCamera)
{
	C_Camera* prev_cull_cam = this->cullingCamera;

	this->cullingCamera = cullingCamera;

	if (prev_cull_cam != nullptr)
	{
		if (prev_cull_cam != cullingCamera)
		{
			prev_cull_cam->SetIsCulling(false);
		}
	}
}

bool M_Scene::GameObjectIsInsideCullingCamera(GameObject* gameObject)
{	
	if (cullingCamera == nullptr)
	{
		//LOG("[ERROR] Scene: There is currently no camera with culling activated!");
		return true;
	}

	bool intersects = cullingCamera->FrustumIntersectsAABB(gameObject->GetAABB());

	return intersects;
}

GameObject* M_Scene::GetGameObjectByUID(uint32 UID)
{
	if (UID == 0)
	{
		LOG("[ERROR] Scene: Could not Get GameObject by UID! Error: Given UID was 0.");
		return nullptr;
	}
	
	for (auto object = gameObjects.cbegin(); object != gameObjects.cend(); ++object)
	{
		if ((*object)->GetUID() == UID)
		{
			return (*object);
		}
	}
	
	return nullptr;
}

GameObject* M_Scene::GetGameObjectByName(const char* name)
{
	if (name == nullptr)
	{
		LOG("[ERROR] Scene: Could not Get GameObject by Name! Error: Given string was nullptr.");
		return nullptr;
	}
	
	for (auto object = gameObjects.cbegin(); object != gameObjects.cend(); ++object)
	{
		if (strcmp((*object)->GetName(), name) == 0)
		{
			return (*object);
		}
	}

	return nullptr;
}

GameObject* M_Scene::GetSelectedGameObject() const
{
	return selectedGameObject;
}

void M_Scene::SetSelectedGameObject(GameObject* gameObject)
{	
	if (gameObject == nullptr)
	{
		if (selectedGameObject != nullptr)
		{
			LOG("[STATUS] Scene: De-Selected %s!", selectedGameObject->GetName());
			selectedGameObject->show_bounding_boxes = false;
		}

		selectedGameObject = nullptr;
	}
	else
	{
		if (gameObject != selectedGameObject)
		{
			if (selectedGameObject != nullptr)
			{
				selectedGameObject->show_bounding_boxes = false;
			}
			
			selectedGameObject = gameObject;
			
			float3 goRef		= gameObject->GetComponent<C_Transform>()->GetWorldPosition();
			float3 reference	= { goRef.x, goRef.y, goRef.z };

			App->camera->SetReference(reference);

			selectedGameObject->show_bounding_boxes = true;
		}
	}	
}

void M_Scene::SelectGameObjectThroughRaycast(const LineSegment& ray)
{	
	std::map<float, GameObject*> hits;
	GetRaycastHits(ray, hits);

	if (hits.size() == 0)																									// --- If no GameObject's AABB was hit
	{
		SetSelectedGameObject(nullptr);
		hits.clear();
		return;
	}

	std::vector<C_Mesh*> cMeshes;
	std::map<float, GameObject*>::iterator item;
	for (item = hits.begin(); item != hits.end(); ++item)
	{
		bool foundMeshes = item->second->GetComponents<C_Mesh>(cMeshes);
		if (!foundMeshes)
		{
			//LOG("[ERROR] Scene: GameObject hit by Raycast did not have any Mesh Component!");								// Right Now GetHits() returns false positives.
			continue;
		}
		
		std::vector<Triangle> faces;
		for (uint m = 0; m < cMeshes.size(); ++m)
		{	
			R_Mesh* rMesh = cMeshes[m]->GetMesh();

			if (rMesh == nullptr)
			{
				continue;
			}
			
			LineSegment localRay = ray;
			localRay.Transform(item->second->GetComponent<C_Transform>()->GetWorldTransform().Inverted());
			
			GetFaces(rMesh->vertices, faces);
			for (uint f = 0; f < faces.size(); ++f)
			{	
				if (localRay.Intersects(faces[f], nullptr, nullptr))
				{
					SetSelectedGameObject(item->second);

					faces.clear();
					cMeshes.clear();
					hits.clear();

					return;
				}
			}

			faces.clear();
		}

		cMeshes.clear();
	}

	SetSelectedGameObject(nullptr);																							// If no GameObject's Mesh was hit
	hits.clear();
}

void M_Scene::GetRaycastHits(const LineSegment& ray, std::map<float, GameObject*>& hits)
{
	for (auto object = gameObjects.cbegin(); object != gameObjects.cend(); ++object)
	{
		float3 position = (*object)->GetComponent<C_Transform>()->GetWorldPosition();
		hits.emplace(ray.Distance(position), (*object));
	}
}

void M_Scene::GetFaces(const std::vector<float>& vertices, std::vector<Triangle>& faces)
{
	std::vector<float3> verts;
	for (uint v = 0; v < vertices.size(); v += 3)
	{
		float v1 = vertices[v];
		float v2 = vertices[v + 1];
		float v3 = vertices[v + 2];

		verts.push_back(float3(v1, v2, v3));

		if (verts.size() == 3)
		{
			faces.push_back(Triangle(verts[0], verts[1], verts[2]));
			verts.clear();
		}
	}

	verts.clear();
}

// --- SCENE LIGHT METHODS
void M_Scene::AddSceneLight(GameObject* light)
{
	if (light == nullptr)
	{
		LOG("[ERROR] Scene: Could not add Light to Scene! Error: Given GameObject* was nullptr.");
		return;
	}
	
	AddGameObjectToVector(light);
}

bool M_Scene::CheckSceneLight()
{
	if (gameObjects.empty())
		return false;
	
	for (auto object = gameObjects.cbegin(); object != gameObjects.cend(); ++object)
	{
		if ((*object)->GetComponent<C_Light>() != nullptr)
			return true;
	}
	
	return false;
}

bool M_Scene::SceneHasLights()
{
	if (gameObjects.empty())
		return false;

	for (auto object = gameObjects.cbegin(); object != gameObjects.cend(); ++object)
	{
		if ((*object)->GetComponent<C_Light>() != nullptr)
			return true;
	}
	
	return false;
}

void M_Scene::GetAllLights(std::vector<GameObject*>& allLights)
{
	if (gameObjects.empty())
		return;
	
	for (auto object = gameObjects.cbegin(); object != gameObjects.cend(); ++object)
	{
		if ((*object)->GetComponent<C_Light>() != nullptr)
			allLights.push_back((*object));
	}
}

void M_Scene::GetDirLights(std::vector<GameObject*>& dirLights)
{
	if (gameObjects.empty())
		return;
	
	for (auto object = gameObjects.cbegin(); object != gameObjects.cend(); ++object)
	{
		C_Light* light = (*object)->GetComponent<C_Light>();
		if (light != nullptr)
		{
			if (light->GetLightType() == LightType::DIRECTIONAL)
				dirLights.push_back((*object));
		}
	}
}

void M_Scene::GetPointLights(std::vector<GameObject*>& pointLights)
{
	if (gameObjects.empty())
		return;

	for (auto object = gameObjects.cbegin(); object != gameObjects.cend(); ++object)
	{
		C_Light* light = (*object)->GetComponent<C_Light>();
		if (light != nullptr)
		{
			if (light->GetLightType() == LightType::POINTLIGHT)
				pointLights.push_back((*object));
		}
	}
}

void M_Scene::NextRoom()
{
	
	nextScene = true;
	
}

void M_Scene::HandleCopyGO() //TODO Cntrl + c / Cntrl + v
{
	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KeyState::KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_C) == KeyState::KEY_DOWN)
	{
		if (selectedGameObject != nullptr)
		{
			//copiedGO = selectedGameObject;
			//LOG("Copied Game Object with Name: %s", copiedGO->GetName());
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KeyState::KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_V) == KeyState::KEY_DOWN)
	{
		//if (selectedGameObject != nullptr && copiedGO != nullptr)
		//{
		//	//AddGameObjectToScene(copiedGO);
		//	//LOG("Pasted Game Object with Name: %s", copiedGO->GetName());
		//}
	}

}

void M_Scene::ResolveScriptGoPointer(const uint32 uid, GameObject** object)
{
	toAdd.push_back({uid, object});
}

void M_Scene::ShowFPS()
{
	/*if (!showFps)
	{
		showFps = new GameObject("FPS Count");
		showFps->CreateComponent(ComponentType::CANVAS);
		showFps->CreateComponent(ComponentType::UI_TEXT);
		gameObjects.push_back(showFps);
	}
	else
	{
		C_UI_Text* fpsCount = (showFps)->GetComponent<C_UI_Text>();
		fpsCount->SetText((char*)App->window->GetRefreshRate());
	}*/
}

void M_Scene::ScriptChangeScene(const std::string& sceneName)
{
	nextScene = true;
	nextSceneName = sceneName;
}

void M_Scene::DeleteSelectedGameObject()
{
	if (selectedGameObject != nullptr)
	{
		DeleteGameObject(selectedGameObject);
	}
}