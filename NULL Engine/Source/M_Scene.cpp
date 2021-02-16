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

#include "M_Scene.h"

#include "MemoryManager.h"

M_Scene::M_Scene(bool is_active) : Module("SceneManager", is_active),
master_root				(nullptr),
scene_root				(nullptr),
animation_root			(nullptr),
selected_game_object	(nullptr),
culling_camera			(nullptr)
{
	CreateMasterRoot();
	CreateSceneRoot("MainScene");

	selected_game_object = scene_root;
}

M_Scene::~M_Scene()
{
	master_root->CleanUp();
	RELEASE(master_root);
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

	if (scene_root == nullptr)
	{
		scene_root = CreateGameObject("Main Scene");

		selected_game_object = scene_root;
	}

	CreateSceneCamera("SceneCamera");

	//uint32 street_uid = App->resource_manager->LoadFromLibrary(DEFAULT_SCENE);
	//GenerateGameObjectsFromModel(street_uid);

	//uint32 animation_uid = App->resource_manager->LoadFromLibrary(DEFAULT_ANIMATION);
	//GenerateGameObjectsFromModel(animation_uid , float3(0.05f, 0.05f, 0.05f));
	
	LoadScene("Assets/Scenes/MainScene.json");
	SaveScene("SceneAutosave");																					// Autosave just right after loading the scene.

	return ret;
}

// Update
UPDATE_STATUS M_Scene::Update(float dt)
{
	if (App->debug == true)
	{
		HandleDebugInput();
	}

	if (animation_root != nullptr)
	{
		C_Animator* root_animator = animation_root->GetComponent<C_Animator>();
		if (root_animator != nullptr)
		{
			if (App->play)
			{
				if (App->input->GetKey(SDL_SCANCODE_KP_1) == KEY_STATE::KEY_DOWN)
				{
					root_animator->PlayClip("Running", 8);
				}
				if (App->input->GetKey(SDL_SCANCODE_KP_1) == KEY_STATE::KEY_UP)
				{
					root_animator->PlayClip("Idle", 8);
				}
				if (App->input->GetKey(SDL_SCANCODE_KP_2) == KEY_STATE::KEY_DOWN)
				{
					root_animator->PlayClip("Attack", 8);
				}

				if (!root_animator->GetCurrentClip()->playing || !root_animator->CurrentClipExists())
				{
					root_animator->PlayClip("Idle", 8);
				}
			}
		}
	}

	std::vector<MeshRenderer>		mesh_renderers;
	std::vector<CuboidRenderer>		cuboid_renderers;
	std::vector<SkeletonRenderer>	skeleton_renderers;

	// --- Sort GameObjects by Z-Buffer value

	for (uint i = 0; i < game_objects.size(); ++i)
	{
		if (game_objects[i]->to_delete)
		{
			DeleteGameObject(game_objects[i], i);
			continue;
		}

		if (game_objects[i]->IsActive())
		{
			game_objects[i]->Update();

			if (GameObjectIsInsideCullingCamera(game_objects[i]) || game_objects[i] == culling_camera->GetOwner())
			{
				game_objects[i]->GetRenderers(mesh_renderers, cuboid_renderers, skeleton_renderers);
			}
		}
	}

	// --- Send Batches to Renderer
	App->renderer->AddRenderersBatch(mesh_renderers, cuboid_renderers, skeleton_renderers);

	mesh_renderers.clear();
	cuboid_renderers.clear();
	
	// --- Primitives
	for (uint n = 0; n < primitives.size(); n++)
	{
		primitives[n]->Update();
	}

	if (App->input->GetKey(SDL_SCANCODE_F5) == KEY_STATE::KEY_DOWN)
	{
		App->SaveConfiguration("Resources/Engine/Configuration/configuration.JSON");
	}

	if (App->input->GetKey(SDL_SCANCODE_F6) == KEY_STATE::KEY_DOWN)
	{
		App->LoadConfiguration("Resources/Engine/Configuration/configuration.JSON");
	}

	return UPDATE_STATUS::CONTINUE;
}

UPDATE_STATUS M_Scene::PostUpdate(float dt)
{
	BROFILER_CATEGORY("M_Scene PostUpdate", Profiler::Color::Yellow)
	
	for (uint n = 0; n < primitives.size(); n++)
	{
		primitives[n]->Render();
	}

	return UPDATE_STATUS::CONTINUE;
}

// Load assets
bool M_Scene::CleanUp()
{
	LOG("Unloading Intro scene");
	
	for (uint i = 0; i < game_objects.size(); ++i)
	{
		game_objects[i]->CleanUp();
		RELEASE(game_objects[i]);
	}

	game_objects.clear();

	scene_root				= nullptr;
	animation_root			= nullptr;
	culling_camera			= nullptr;
	selected_game_object	= nullptr;

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
bool M_Scene::SaveScene(const char* scene_name) const
{
	bool ret = true;

	ParsonNode root_node		= ParsonNode();
	ParsonArray object_array	= root_node.SetArray("Game Objects");

	for (uint i = 0; i < game_objects.size(); ++i)
	{
		ParsonNode array_node = object_array.SetNode(game_objects[i]->GetName());
		game_objects[i]->SaveState(array_node);
	}

	char* buffer		= nullptr;
	std::string name	= (scene_name != nullptr) ? scene_name : scene_root->GetName();
	std::string path	= ASSETS_SCENES_PATH + name + JSON_EXTENSION;
	uint written = root_node.SerializeToFile(path.c_str(), &buffer);
	if (written > 0)
	{
		LOG("[SCENE] Scene: Successfully saved the current scene! Path: %s", path.c_str());
	}
	else
	{
		LOG("[ERROR] Scene: Could not save the current scene! Error: FileSystem could not write any data!");
	}

	R_Scene* r_scene			= (R_Scene*)App->resource_manager->CreateResource(RESOURCE_TYPE::SCENE, path.c_str());			// TMP until R_Scene is fully implemented.
	//App->resource_manager->SaveResourceToLibrary(r_scene);

	std::string library_path	= SCENES_PATH + std::to_string(r_scene->GetUID()) + SCENES_EXTENSION;
	written = App->file_system->Save(library_path.c_str(), buffer, written);
	if (written > 0)
	{
		LOG("[SCENE] Scene: Successfully saved the current scene in Library! Path: %s", library_path.c_str());
	}
	else
	{
		LOG("[ERROR] Scene: Could not save the current scene in Library! Error: FileSystem could not write any data!");
	}

	App->resource_manager->DeallocateResource(r_scene);

	RELEASE_ARRAY(buffer);

	return ret;
}

bool M_Scene::LoadScene(const char* path)
{
	bool ret = true;

	char* buffer = nullptr;
	uint read = App->file_system->Load(path, &buffer);
	if (read == 0)
	{
		LOG("[ERROR] Scene Loading: Could not load %s from Assets! Error: File system could not read the file!", path);
		return false;
	}

	if (buffer != nullptr)
	{
		CleanUp();

		ParsonNode new_root			= ParsonNode(buffer);
		ParsonArray objects_array	= new_root.GetArray("Game Objects");
		RELEASE_ARRAY(buffer);

		std::map<uint32, GameObject*> tmp;

		for (uint i = 0; i < objects_array.size; ++i)																			// Getting all the GameObjects in the ParsonArray
		{
			ParsonNode object_node = objects_array.GetNode(i);
			if (!object_node.NodeIsValid())
			{
				continue;
			}

			GameObject* game_object = new GameObject();

			game_object->LoadState(object_node);

			if (game_object->is_scene_root)
			{
				scene_root = game_object;
				scene_root->SetParent(master_root);
			}

			if (game_object->GetComponent<C_Animator>() != nullptr)
			{
				animation_root = game_object;
			}

			C_Camera* c_camera = game_object->GetComponent<C_Camera>();
			if (c_camera != nullptr)
			{
				if (c_camera->IsCulling())
				{
					culling_camera = c_camera;
				}
			}

			tmp.emplace(game_object->GetUID(), game_object);
		}

		// Re-Parenting
		std::map<uint32, GameObject*>::iterator item;
		for (item = tmp.begin(); item != tmp.end(); ++item)
		{
			uint parent_uid = item->second->GetParentUID();
			if (parent_uid == 0)
			{
				continue;
			}

			std::map<uint32, GameObject*>::iterator parent = tmp.find(parent_uid);
			if (parent != tmp.end())
			{
				item->second->SetParent(parent->second);
			}

			item->second->GetComponent<C_Transform>()->Translate(float3::zero);						// Dirty way to refresh the transforms after the import is done. TMP Un-hardcode later.
			game_objects.push_back(item->second);
		}

		tmp.clear();
		App->renderer->ClearRenderers();
	}

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
	case::RESOURCE_TYPE::MODEL:		{ GenerateGameObjectsFromModel(resource->GetUID()); }				break;
	case::RESOURCE_TYPE::TEXTURE:	{ success = ApplyTextureToSelectedGameObject(resource->GetUID()); }	break;
	}
}

GameObject* M_Scene::CreateGameObject(const char* name, GameObject* parent)
{	
	if (game_objects.empty())
	{
		CreateSceneRoot(name);
		return scene_root;
	}
	
	GameObject* game_object = new GameObject(name);

	if (game_object != nullptr)
	{
		if (parent != nullptr)
		{
			game_object->SetParent(parent);
			
			// parent->AddChild(game_object);
		}

		game_objects.push_back(game_object);
	}

	return game_object;
}

void M_Scene::DeleteGameObject(GameObject* game_object, uint index)
{
	if (game_object == nullptr)
	{
		LOG("[ERROR] Scene: Object to delete was nullptr!");
		return;
	}

	if (selected_game_object == game_object)
	{
		selected_game_object = nullptr;
	}
	
	std::vector<C_Mesh*> c_meshes;
	bool found_meshes = game_object->GetComponents<C_Mesh>(c_meshes);
	if (found_meshes)
	{
		for (uint i = 0; i < c_meshes.size(); ++i)
		{
			App->renderer->DeleteFromMeshRenderers(c_meshes[i]);
		}

		c_meshes.clear();
	}

	if (!game_objects.empty())													// Extra check just to make sure that at least one GameObject remains in the Scene.
	{
		game_object->CleanUp();													// As it has not been Cleaned Up by its parent, the GameObject needs to call its CleanUp();
		
		if (index != -1)														// If an index was given.
		{
			game_objects.erase(game_objects.begin() + index);					// Delete game object at index.
		}
		else
		{
			for (uint i = 0; i < game_objects.size(); ++i)						// If no index was given.
			{
				if (game_objects[i] == game_object)								// Iterate game_objects until a match is found.
				{
					game_objects.erase(game_objects.begin() + i);				// Delete the game_object at the current loop index.
					break;
				}
			}
		}

		RELEASE(game_object);
		return;
	}

	LOG("[ERROR] Could not find game object %s in game_objects vector!", game_object->GetName());
}

void M_Scene::GenerateGameObjectsFromModel(const uint32& model_UID, const float3& scale)
{
	R_Model* r_model = (R_Model*)App->resource_manager->RequestResource(model_UID);

	if (r_model == nullptr)
	{
		LOG("[ERROR] Scene: Could not generate GameObjects from Model Resource! Error: R_Model* was nullptr.");
		return;
	}

	GameObject* parent_root = nullptr;
	std::map<uint32, GameObject*> tmp;

	std::vector<ModelNode> m_nodes = r_model->model_nodes;
	for (uint i = 0; i < m_nodes.size(); ++i)
	{
		GameObject* game_object = new GameObject();

		game_object->ForceUID(m_nodes[i].uid);
		game_object->SetParentUID(m_nodes[i].parent_uid);
		game_object->SetName(m_nodes[i].name.c_str());
		game_object->GetComponent<C_Transform>()->ImportTransform(m_nodes[i].transform);

		CreateComponentsFromModelNode(m_nodes[i], game_object);

		if (m_nodes[i].parent_uid == 0)
		{
			parent_root = game_object;
		}

		tmp.emplace(game_object->GetUID(), game_object);
	}
	  
	m_nodes.clear();

	// Re-Parenting
	std::map<uint32, GameObject*>::iterator item;
	for (item = tmp.begin(); item != tmp.end(); ++item)
	{
		uint parent_uid = item->second->GetParentUID();
		if (parent_uid == 0)
		{
			item->second->SetParent(scene_root);
		}
		else
		{
			std::map<uint32, GameObject*>::iterator parent = tmp.find(parent_uid);
			if (parent != tmp.end())
			{
				item->second->SetParent(parent->second);
			}
		}

		item->second->GetComponent<C_Transform>()->Translate(float3::zero);						// Dirty way to refresh the transforms after the import is done. TMP Un-hardcode later.
		game_objects.push_back(item->second);
	}

	if (parent_root != nullptr)
	{
		if (scale.x != 0.0f)																	// Dirty way of detecting that the scale is valid.
		{
			parent_root->GetComponent<C_Transform>()->SetLocalScale(scale);
		}

		if (!r_model->animations.empty())
		{
			CreateAnimationComponentFromModel(r_model, parent_root);							// Must be done last as the parent hierarchy needs to be in place.
		}

		parent_root = nullptr;
	}

	tmp.clear();
}

void M_Scene::CreateComponentsFromModelNode(const ModelNode& model_node, GameObject* game_object)
{
	bool valid_mesh_uid			= (model_node.mesh_uid != 0)		?	true : false;
	bool valid_material_uid		= (model_node.material_uid != 0)	?	true : false;
	bool valid_texture_uid		= (model_node.texture_uid != 0)		?	true : false;
	
	// Set Mesh
	if (valid_mesh_uid)
	{
		C_Mesh* c_mesh = (C_Mesh*)game_object->CreateComponent(COMPONENT_TYPE::MESH);
		R_Mesh* r_mesh = (R_Mesh*)App->resource_manager->RequestResource(model_node.mesh_uid);
		if (r_mesh == nullptr)
		{
			LOG("[ERROR] Scene: Could not generate the Mesh Resource from the Model Node! Error: R_Mesh* could not be found in resources.");
			game_object->DeleteComponent(c_mesh);
			return;
		}

		c_mesh->SetMesh(r_mesh);
	}

	// Set Material
	if (valid_material_uid)
	{
		C_Material* c_material = (C_Material*)game_object->CreateComponent(COMPONENT_TYPE::MATERIAL);
		R_Material* r_material = (R_Material*)App->resource_manager->RequestResource(model_node.material_uid);
		if (r_material == nullptr)
		{
			LOG("[ERROR] Scene: Could not generate the Material Resource from the Model Node! Error: R_Material* could not be found in resources.");
			game_object->DeleteComponent(c_material);
			return;
		}

		c_material->SetMaterial(r_material);

		// Set Texture
		if (valid_texture_uid)
		{
			R_Texture* r_texture = (R_Texture*)App->resource_manager->RequestResource(model_node.texture_uid);
			if (r_texture == nullptr)
			{
				LOG("[ERROR] Scene: Could not generate the Texture Resource from the Model Node! Error: R_Texture* could not be found in resources.");
				return;
			}

			c_material->SetTexture(r_texture);
		}
	}
}

void M_Scene::CreateAnimationComponentFromModel(const R_Model* r_model, GameObject* game_object)
{
	if (r_model == nullptr)
	{
		LOG("[ERROR] Scene: Could not Create Animation Component From Model! Error: Given R_Model* was nullptr.");
		return;
	}
	if (game_object == nullptr)
	{
		LOG("[ERROR] Scene: Could not Create Animation Component From Model! Error: Given GameObject* was nullptr.");
		return;
	}
	if (r_model->animations.empty())
	{
		LOG("[ERROR] Scene: Could not Create Animation Component From Model! Error: Given R_Model* had no animations.");
		return;
	}

	animation_root = game_object;

	C_Animator* c_animation = (C_Animator*)game_object->CreateComponent(COMPONENT_TYPE::ANIMATOR);
	std::map<uint32, std::string>::const_iterator item;
	for (item = r_model->animations.cbegin(); item != r_model->animations.cend(); ++item)
	{
		R_Animation* r_animation = (R_Animation*)App->resource_manager->RequestResource(item->first);
		if (r_animation != nullptr)
		{
			c_animation->AddAnimation(r_animation);
		}
	}
}

std::vector<GameObject*>* M_Scene::GetGameObjects()
{
	return &game_objects;
}

bool M_Scene::ApplyTextureToSelectedGameObject(const uint32& UID)
{
	BROFILER_CATEGORY("ApplyNewTextureToSelectedGameObject()", Profiler::Color::Magenta);

	if (selected_game_object == nullptr)
	{
		LOG("[ERROR] Could not add the texture to the selected game object! Error: No game object was being selected.");
		return false;
	}
	if (UID == 0)
	{
		LOG("[ERROR] Could not add the texture to the selected game object! Error: Given Resource UID was 0.");
		return false;
	}

	R_Texture* r_texture = (R_Texture*)App->resource_manager->RequestResource(UID);

	if (r_texture == nullptr)
	{
		LOG("[ERROR] Could not add the texture to the selected game object! Error: R_Texture* was nullptr.");
		return false;
	}
	if (r_texture->GetTextureID() == 0)
	{
		LOG("[ERROR] Could not add the texture to the selected game object! Error: R_Texture* Texture ID was 0.");
		App->resource_manager->DeallocateResource(r_texture);
		return false;
	}

	// --- SETTING THE NEW TEXTURE ---
	C_Material* c_material = selected_game_object->GetComponent<C_Material>();									// GetMaterialComponent() == nullptr if GO does not have a C_Material.
	if (c_material == nullptr)
	{
		c_material = (C_Material*)selected_game_object->CreateComponent(COMPONENT_TYPE::MATERIAL);				// Creating a Material Component if none was found in the selected GO.
	}

	c_material->SetTexture(r_texture);																			// Setting the Material Component's texture with the newly created one.

	return true;
}

void M_Scene::CreateMasterRoot()
{
	master_root = new GameObject("MasterRoot");
	master_root->is_master_root = true;
}

void M_Scene::DeleteMasterRoot()
{
	if (master_root != nullptr)
	{
		master_root->CleanUp();
		RELEASE(master_root);
	}
}

GameObject* M_Scene::GetMasterRoot() const
{
	return master_root;
}

void M_Scene::CreateSceneRoot(const char* scene_name)
{
	if (master_root == nullptr)
	{
		CreateMasterRoot();																	// Safety Check.
	}
	
	scene_root = new GameObject(scene_name);

	scene_root->is_scene_root = true;

	scene_root->SetParent(master_root);
	
	//scene_root->parent = master_root;
	//master_root->AddChild(scene_root);

	game_objects.push_back(scene_root);
}

GameObject* M_Scene::GetSceneRoot() const
{
	return scene_root;
}

void M_Scene::SetSceneRoot(GameObject* game_object)
{
	if (game_object != scene_root)
	{
		scene_root = game_object;
	}
}

void M_Scene::ChangeSceneName(const char* name)
{
	scene_root->SetName(name);
}

void M_Scene::CreateSceneCamera(const char* camera_name)
{
	GameObject* scene_camera = CreateGameObject(camera_name, scene_root);
	scene_camera->CreateComponent(COMPONENT_TYPE::CAMERA);
	scene_camera->GetComponent<C_Camera>()->SetAspectRatio((float)App->window->GetWidth() / (float)App->window->GetHeight());
	scene_camera->GetComponent<C_Transform>()->SetLocalPosition(float3(0.0f, 5.0f, 25.0f));
}

C_Camera* M_Scene::GetCullingCamera() const
{
	return culling_camera;
}

void M_Scene::SetCullingCamera(C_Camera* culling_camera)
{
	C_Camera* prev_cull_cam = this->culling_camera;

	this->culling_camera = culling_camera;

	if (prev_cull_cam != nullptr)
	{
		if (prev_cull_cam != culling_camera)
		{
			prev_cull_cam->SetIsCulling(false);
		}
	}
}

bool M_Scene::GameObjectIsInsideCullingCamera(GameObject* game_object)
{
	if (culling_camera == nullptr)
	{
		//LOG("[ERROR] Scene: There is currently no camera with culling activated!");
		return true;
	}

	bool intersects = culling_camera->FrustumIntersectsAABB(game_object->GetAABB());

	return intersects;
}

GameObject* M_Scene::GetSelectedGameObject() const
{
	return selected_game_object;
}

void M_Scene::SetSelectedGameObject(GameObject* game_object)
{	
	if (game_object == nullptr)
	{
		if (selected_game_object != nullptr)
		{
			LOG("[STATUS] Scene: De-Selected %s!", selected_game_object->GetName());
			selected_game_object->show_bounding_boxes = false;
		}

		selected_game_object = nullptr;
	}
	else
	{
		if (game_object != selected_game_object)
		{
			if (selected_game_object != nullptr)
			{
				selected_game_object->show_bounding_boxes = false;
			}
			
			selected_game_object = game_object;
			
			float3 go_ref		= game_object->GetComponent<C_Transform>()->GetWorldPosition();
			float3 reference	= { go_ref.x, go_ref.y, go_ref.z };

			App->camera->SetReference(reference);

			selected_game_object->show_bounding_boxes = true;
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

	std::vector<C_Mesh*> c_meshes;
	std::map<float, GameObject*>::iterator item;
	for (item = hits.begin(); item != hits.end(); ++item)
	{
		bool found_meshes = item->second->GetComponents<C_Mesh>(c_meshes);
		if (!found_meshes)
		{
			//LOG("[ERROR] Scene: GameObject hit by Raycast did not have any Mesh Component!");								// Right Now GetHits() returns false positives.
			continue;
		}
		
		std::vector<Triangle> faces;
		for (uint m = 0; m < c_meshes.size(); ++m)
		{	
			R_Mesh* r_mesh = c_meshes[m]->GetMesh();

			if (r_mesh == nullptr)
			{
				continue;
			}
			
			LineSegment local_ray = ray;
			local_ray.Transform(item->second->GetComponent<C_Transform>()->GetWorldTransform().Inverted());
			
			GetFaces(r_mesh->vertices, faces);
			for (uint f = 0; f < faces.size(); ++f)
			{	
				if (local_ray.Intersects(faces[f], nullptr, nullptr))
				{
					SetSelectedGameObject(item->second);

					faces.clear();
					c_meshes.clear();
					hits.clear();

					return;
				}
			}

			faces.clear();
		}

		c_meshes.clear();
	}

	SetSelectedGameObject(nullptr);																							// If no GameObject's Mesh was hit
	hits.clear();
}

void M_Scene::GetRaycastHits(const LineSegment& ray, std::map<float, GameObject*>& hits)
{
	for (uint i = 0; i < game_objects.size(); ++i)
	{
		if (ray.Intersects(game_objects[i]->GetAABB()))
		{
			float3 position = game_objects[i]->GetComponent<C_Transform>()->GetWorldPosition();
			hits.emplace(ray.Distance(position), game_objects[i]);
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

void M_Scene::DeleteSelectedGameObject()
{
	if (selected_game_object != nullptr)
	{
		DeleteGameObject(selected_game_object);
	}
}

void M_Scene::HandleDebugInput()
{
	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_STATE::KEY_DOWN)
	{
		//DebugSpawnPrimitive(new Sphere(1.0f, 12, 24));
	}

	if (App->input->GetKey(SDL_SCANCODE_2) == KEY_STATE::KEY_DOWN)
	{
		DebugSpawnPrimitive(new P_Cube());
	}

	if (App->input->GetKey(SDL_SCANCODE_3) == KEY_STATE::KEY_DOWN)
	{
		//DebugSpawnPrimitive(new Cylinder());
	}

	if (App->input->GetKey(SDL_SCANCODE_4) == KEY_STATE::KEY_DOWN)
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