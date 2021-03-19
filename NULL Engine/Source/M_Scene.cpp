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

#include "Resource.h"
#include "R_Model.h"
#include "R_Mesh.h"
#include "R_Texture.h"
#include "R_Scene.h"

#include "Primitive.h"

#include "GameObject.h"
#include "Component.h"
#include "C_Transform.h"
#include "C_Mesh.h"
#include "C_Material.h"
#include "C_Camera.h"
#include "C_Animator.h"
#include "C_Light.h"

#include "M_Scene.h"

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

	level.GetRooms();
	level.GenerateLevel();
	
	//level.AddFixedRoom("Shop", 12);
	//level.AddFixedRoom("Boss", 20); 
	
	level.GenerateRoom(0);

	//LoadScene("Assets/Scenes/UITestScene.json");
	//SaveScene("SceneAutosave");																			// Autosave just right after loading the scene.

	return ret;
}

// Update
UpdateStatus M_Scene::Update(float dt)
{
	OPTICK_CATEGORY("Scene Update", Optick::Category::Update);
	
	if (App->debug == true)
	{
		HandleDebugInput();
	}

	/*if (animationRoot != nullptr)
	{
		C_Animator* rootAnimator = animationRoot->GetComponent<C_Animator>();
		if (rootAnimator != nullptr)
		{
			if (App->gameState == GameState::PLAY)
			{
				if (App->input->GetKey(SDL_SCANCODE_KP_1) == KeyState::KEY_DOWN)
				{
					rootAnimator->PlayClip("Running", 8);
				}
				if (App->input->GetKey(SDL_SCANCODE_KP_1) == KeyState::KEY_UP)
				{
					rootAnimator->PlayClip("Idle", 8);
				}
				if (App->input->GetKey(SDL_SCANCODE_KP_2) == KeyState::KEY_DOWN)
				{
					rootAnimator->PlayClip("Attack", 8);
				}

				if (!rootAnimator->CurrentClipExists() || !rootAnimator->GetCurrentClip()->playing)
				{
					rootAnimator->PlayClip("Idle", 8);
				}
			}
		}
	}*/

	std::vector<MeshRenderer>		meshRenderers;
	std::vector<CuboidRenderer>		cuboidRenderers;
	std::vector<SkeletonRenderer>	skeletonRenderers;

	// --- Sort GameObjects by Z-Buffer value

	for (uint i = 0; i < gameObjects.size(); ++i)
	{
		if (gameObjects[i]->to_delete)
		{
			DeleteGameObject(gameObjects[i], i);
			continue;
		}

		if (gameObjects[i]->IsActive())
		{
			gameObjects[i]->Update();

			if (GameObjectIsInsideCullingCamera(gameObjects[i]) || gameObjects[i] == cullingCamera->GetOwner())
			{
				gameObjects[i]->GetRenderers(meshRenderers, cuboidRenderers, skeletonRenderers);
			}
		}
	}

	// --- Send Batches to Renderer
	App->renderer->AddRenderersBatch(meshRenderers, cuboidRenderers, skeletonRenderers);

	meshRenderers.clear();
	cuboidRenderers.clear();
	
	// --- Primitives
	for (uint n = 0; n < primitives.size(); n++)
	{
		primitives[n]->Update();
	}

	if (App->input->GetKey(SDL_SCANCODE_F5) == KeyState::KEY_DOWN)
	{
		App->SaveConfiguration("Resources/Engine/Configuration/configuration.JSON");
	}

	if (App->input->GetKey(SDL_SCANCODE_F6) == KeyState::KEY_DOWN)
	{
		App->LoadConfiguration("Resources/Engine/Configuration/configuration.JSON");
	}

	// --- Room Generation

	level.HandleRoomGeneration();

	return UpdateStatus::CONTINUE;
}

UpdateStatus M_Scene::PostUpdate(float dt)
{
	if (nextScene)
	{
		level.NextRoom();
		nextScene = false;
	}

	for (uint n = 0; n < primitives.size(); n++)
	{
		primitives[n]->Render();
	}

	return UpdateStatus::CONTINUE;
}

// Load assets
bool M_Scene::CleanUp()
{
	LOG("Unloading Intro scene");
	
	//App->renderer->ClearRenderers();

	App->renderer->defaultSkyBox.CleanUp();

	for (uint i = 0; i < gameObjects.size(); ++i)
	{
		gameObjects[i]->CleanUp();
		//gameObjects.erase(gameObjects.begin() + i);
		RELEASE(gameObjects[i]);
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

	primitives.clear();

	return true;
}

bool M_Scene::SaveConfiguration(ParsonNode& root) const
{
	bool ret = true;

	//SaveScene();

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

	ParsonArray objectArray	= rootNode.SetArray("Game Objects");
	for (auto item = gameObjects.begin(); item != gameObjects.end(); ++item)
	{
		ParsonNode arrayNode = objectArray.SetNode((*item)->GetName());
		(*item)->SaveState(arrayNode);
	}

	/*for (uint i = 0; i < gameObjects.size(); ++i)
	{
		ParsonNode array_node = objectArray.SetNode(gameObjects[i]->GetName());
		gameObjects[i]->SaveState(array_node);
	}*/

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

	App->camera->SetMasterCameraAsCurrentCamera();

	char* buffer = nullptr;
	uint read = App->fileSystem->Load(path, &buffer);
	if (read == 0)
	{
		LOG("[ERROR] Scene Loading: Could not load %s from Assets! Error: File system could not read the file!", path);
		return false;
	}

	if (buffer != nullptr)
	{
		App->renderer->ClearRenderers();
		CleanUp();

		ParsonNode newRoot			= ParsonNode(buffer);
		ParsonArray modelsArray		= newRoot.GetArray("Models In Scene");
		ParsonArray objectsArray	= newRoot.GetArray("Game Objects");
		RELEASE_ARRAY(buffer);

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

			if (gameObject->is_scene_root)
			{
				sceneRoot = gameObject;
				sceneRoot->SetParent(masterRoot);
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

		// Re-Parenting
		std::map<uint32, GameObject*>::iterator item;
		for (item = tmp.begin(); item != tmp.end(); ++item)
		{
			uint parentUid = item->second->GetParentUID();
			if (parentUid == 0)
			{
				continue;
			}

			std::map<uint32, GameObject*>::iterator parent = tmp.find(parentUid);
			if (parent != tmp.end())
			{
				item->second->SetParent(parent->second);
			}

			item->second->GetComponent<C_Transform>()->Translate(float3::zero);						// Dirty way to refresh the transforms after the import is done. TMP Un-hardcode later.
			gameObjects.push_back(item->second);
		}
		tmp.clear();
		App->renderer->ClearRenderers();
	}

	//FIX THIS
	App->renderer->defaultSkyBox.SetUpSkyBoxBuffers();

	LOG("Successfully Loaded Scene: %s", path);

	//if (!CheckSceneLight()) AddSceneLight(App->renderer->GenerateSceneLight(Color(1.0f, 1.0f, 1.0f, 1.0f), Color(0.6, 0.6, 0.6, 0.5), Color(0.6, 0.6, 0.6, 0.5), LightType::DIRECTIONAL));

	return ret;
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

void M_Scene::LoadPrefabIntoScene(ParsonNode* a)
{
	GameObject* gameObject = new GameObject();

	gameObject->LoadState(*a);

	gameObject->SetParent(App->scene->GetSceneRoot());

	gameObjects.push_back(gameObject);

	ParsonArray childArray = a->GetArray("Children");

	for (int i = 0; i < childArray.size; i++)
	{
		App->scene->LoadPrefabObject(gameObject, &childArray.GetNode(i));
	}
}

void M_Scene::LoadPrefabObject(GameObject* _gameObject, ParsonNode* node)
{
	GameObject* gameObject = new GameObject();

	gameObject->LoadState(*node);

	gameObject->SetParent(_gameObject);

	gameObjects.push_back(gameObject);

	ParsonArray childArray = node->GetArray("Children");
	for (int i = 0; i < childArray.size; i++)
	{
		LoadPrefabObject(gameObject, &childArray.GetNode(i));
	}
}

GameObject* M_Scene::CreateGameObject(const char* name, GameObject* parent)
{	
	if (gameObjects.empty())
	{
		CreateSceneRoot(name);
		return sceneRoot;
	}
	
	GameObject* gameObject = new GameObject(name);

	if (gameObject != nullptr)
	{
		if (parent != nullptr)
		{
			gameObject->SetParent(parent);
			
			// parent->AddChild(game_object);
		}

		gameObjects.push_back(gameObject);
	}

	return gameObject;
}

void M_Scene::DeleteGameObject(GameObject* gameObject, uint index)
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
	bool found_meshes = gameObject->GetComponents<C_Mesh>(cMeshes);
	if (found_meshes)
	{
		for (uint i = 0; i < cMeshes.size(); ++i)
		{
			App->renderer->DeleteFromMeshRenderers(cMeshes[i]);
		}

		cMeshes.clear();
	}

	if (!gameObjects.empty())													// Extra check just to make sure that at least one GameObject remains in the Scene.
	{
		gameObject->CleanUp();													// As it has not been Cleaned Up by its parent, the GameObject needs to call its CleanUp();
		
		if (index != -1)														// If an index was given.
		{
			gameObjects.erase(gameObjects.begin() + index);					// Delete game object at index.
		}
		else
		{
			for (uint i = 0; i < gameObjects.size(); ++i)					// If no index was given.
			{
				if (gameObjects[i] == gameObject)							// Iterate game_objects until a match is found.
				{
					gameObjects.erase(gameObjects.begin() + i);				// Delete the game_object at the current loop index.
					break;
				}
			}
		}

		RELEASE(gameObject);
		return;
	}

	LOG("[ERROR] Could not find game object %s in game_objects vector!", gameObject->GetName());
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
		gameObjects.push_back(item->second);
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

		//parentRoot = nullptr; //I David Rami purposelly and for a reason comented this
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
	masterRoot->is_master_root = true;
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

void M_Scene::CreateSceneRoot(const char* sceneName)
{
	if (masterRoot == nullptr)
	{
		CreateMasterRoot();																	// Safety Check.
	}
	
	sceneRoot = new GameObject(sceneName);

	sceneRoot->is_scene_root = true;

	sceneRoot->SetParent(masterRoot);
	
	//scene_root->parent = master_root;
	//master_root->AddChild(scene_root);

	gameObjects.push_back(sceneRoot);
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
	for (uint i = 0; i < gameObjects.size(); ++i)
	{
		if (ray.Intersects(gameObjects[i]->GetAABB()))
		{
			float3 position = gameObjects[i]->GetComponent<C_Transform>()->GetWorldPosition();
			hits.emplace(ray.Distance(position), gameObjects[i]);
		}
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

bool M_Scene::CheckSceneLight()
{
	for (int i = 0; i < gameObjects.size(); i++)
	{
		if (gameObjects[i]->GetComponent<C_Light>())
		{
			return true;
		}
	}
	
	return false;
}

std::vector<GameObject*> M_Scene::GetAllLights()
{
	std::vector<GameObject*> allLights;
	for (uint i = 0; i < gameObjects.size(); i++)
	{
		if (gameObjects[i]->GetComponent<C_Light>())
		{
			allLights.push_back(gameObjects[i]);
		}
		
	}
	return allLights;
}

void M_Scene::AddSceneLight(GameObject* light)
{
	gameObjects.push_back(light);
}

std::vector<GameObject*> M_Scene::GetDirLights()
{
	std::vector<GameObject*> dirLights;
	for (uint i = 0; i < gameObjects.size(); i++)
	{
		if (gameObjects[i]->GetComponent<C_Light>())
		{
			if (gameObjects[i]->GetComponent<C_Light>()->GetLightType() == LightType::DIRECTIONAL)
			{
				dirLights.push_back(gameObjects[i]);
			}
		}
		
	}
	return dirLights;
}

std::vector<GameObject*> M_Scene::GetPointLights()
{
	std::vector<GameObject*> pointLights;
	for (uint i = 0; i < gameObjects.size(); i++)
	{
		if (gameObjects[i]->GetComponent<C_Light>())
		{
			if (gameObjects[i]->GetComponent<C_Light>()->GetLightType() == LightType::POINTLIGHT)
			{
				pointLights.push_back(gameObjects[i]);
			}
		}
	}
	return pointLights;
}

void M_Scene::NextRoom()
{
	nextScene = true;
}

void M_Scene::DeleteSelectedGameObject()
{
	if (selectedGameObject != nullptr)
	{
		DeleteGameObject(selectedGameObject);
	}
}

void M_Scene::HandleDebugInput()
{
	if (App->input->GetKey(SDL_SCANCODE_1) == KeyState::KEY_DOWN)
	{
		//DebugSpawnPrimitive(new Sphere(1.0f, 12, 24));
	}

	if (App->input->GetKey(SDL_SCANCODE_2) == KeyState::KEY_DOWN)
	{
		DebugSpawnPrimitive(new P_Cube());
	}

	if (App->input->GetKey(SDL_SCANCODE_3) == KeyState::KEY_DOWN)
	{
		//DebugSpawnPrimitive(new Cylinder());
	}

	if (App->input->GetKey(SDL_SCANCODE_4) == KeyState::KEY_DOWN)
	{
		for (uint n = 0; n < primitives.size(); n++)
		{
			primitives[n]->SetPos((float)(std::rand() % 40 - 20), 10.f, (float)(std::rand() % 40 - 20));
		}
	}

	//if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_DOWN)
	//{
	//	//Get a vector indicating the direction from the camera viewpoint to the "mouse"
	//	float mouse_x_position = ((float)App->input->GetMouseX() / (float)App->window->GetWidth()) * 2.f - 1.f;
	//	float mouse_y_position = -((float)App->input->GetMouseY() / (float)App->window->GetHeight()) * 2.f + 1.f;
	//
	//	/*const float2 mouse_pos(mouse_x_position, mouse_y_position);
	//
	//	const float4 ray_eye = App->renderer->GetProjectionMatrix().Inverted() * float4(mouse_pos.x, mouse_pos.y, -1.f, 1.f);
	//	const float4 ray_world(App->camera->GetViewMatrix().Inverted() * float4(ray_eye.x, ray_eye.y, -1.f, 0.f));*/
	//
	//	const vec2 mouse_pos(mouse_x_position, mouse_y_position);
	//
	//	const vec4 ray_eye = inverse(App->renderer->GetProjectionMatrix()) * vec4(mouse_pos.x, mouse_pos.y, -1.f, 1.f);
	//	const vec4 ray_world(inverse(App->camera->GetViewMatrix()) * vec4(ray_eye.x, ray_eye.y, -1.f, 0.f));
	//
	//	float3 dir(ray_world.x, ray_world.y, ray_world.z);
	//}
}

void M_Scene::DebugSpawnPrimitive(Primitive* p)
{
	//primitives.push_back(p);
	//p->SetPos(App->camera->position.x, App->camera->position.y, App->camera->position.z);
}
