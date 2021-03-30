#include "Profiler.h"

#include "VariableTypedefs.h"
#include "Macros.h"
#include "Log.h"

#include "JSONParser.h"
#include "Random.h"

#include "Application.h"
#include "M_Renderer3D.h"																				// TMP. Move the Renderers generation elsewhere.

#include "M_Camera3D.h"																					//TEMP. putting the cam if game time

#include "R_Mesh.h"

#include "Component.h"
#include "C_Transform.h"
#include "C_Mesh.h"
#include "C_Material.h"
#include "C_Light.h"
#include "C_Camera.h"
#include "C_Animator.h"
#include "C_Animation.h"
#include "C_AudioSource.h"
#include "C_AudioListener.h"
#include "C_Script.h"
#include "C_RigidBody.h"
#include "C_BoxCollider.h"
#include "C_SphereCollider.h"
#include "C_CapsuleCollider.h"
#include "C_PlayerController.h"
#include "C_BulletBehavior.h"
#include "C_PropBehavior.h"
#include "C_CameraBehavior.h"
#include "C_GateBehavior.h"

#include "C_ParticleSystem.h"

#include "C_Canvas.h"

#include "C_UI_Image.h"
#include "C_UI_Text.h"


#include "GameObject.h"

#include "MemoryManager.h"

GameObject::GameObject() :
uid					(Random::LCG::GetRandomUint()),
parent_uid			(0),
name				("GameObject"),
isActive			(true),
isStatic			(false),
parent				(nullptr),
transform			(nullptr),
is_master_root		(false),
is_scene_root		(false),
is_bone				(false),
to_delete			(false),
show_bounding_boxes	(false)
{
	transform = (C_Transform*)CreateComponent(ComponentType::TRANSFORM);

	obb.SetNegativeInfinity();
	aabb.SetNegativeInfinity();

	obb_vertices	= new float3[8];																		// Bounding boxes will always have 8 vertices as they are Cuboids.
	aabb_vertices	= new float3[8];																		// Bounding boxes will always have 8 vertices as they are Cuboids.
}

GameObject::GameObject(std::string name, bool isActive, bool isStatic) :
uid					(Random::LCG::GetRandomUint()),
parent_uid			(0),
name				(name),
isActive			(isActive),
isStatic			(isStatic),
parent				(nullptr),
transform			(nullptr),
is_master_root		(false),
is_scene_root		(false),
is_bone				(false),
to_delete			(false)
{
	if (name.empty())
	{
		name = "GameObject";
	}

	transform = (C_Transform*)CreateComponent(ComponentType::TRANSFORM);

	obb.SetNegativeInfinity();
	aabb.SetNegativeInfinity();

	obb_vertices	= new float3[8];																		// Bounding boxes will always have 8 vertices as they are Cuboids.
	aabb_vertices	= new float3[8];																		// Bounding boxes will always have 8 vertices as they are Cuboids.
}

GameObject::~GameObject()
{
	RELEASE_ARRAY(obb_vertices);
	RELEASE_ARRAY(aabb_vertices);
}

bool GameObject::Update()
{
	bool ret = true;

	for (uint i = 0; i < components.size(); ++i)
	{
		if (components[i]->IsActive())
		{
			components[i]->Update();
		}
	}

	UpdateBoundingBoxes();																					// Make the call in C_Transform after receiving a dirty flag?

	return ret;
}

bool GameObject::CleanUp()
{
	bool ret = true;

	FreeComponents();

	FreeChilds();

	return ret;
}

bool GameObject::SaveState(ParsonNode& root) const
{
	bool ret = true;

	root.SetNumber("UID", uid);

	uint parentUID = (parent != nullptr) ? parent->uid : 0;
	root.SetNumber("ParentUID", parentUID);

	root.SetNumber("PrefabID", prefabID);
	root.SetBool("IsPrefab", isPrefab);

	root.SetString("Name", name.c_str());
	root.SetBool("IsActive", isActive);
	root.SetBool("IsStatic", isStatic);
	root.SetBool("IsSceneRoot", is_scene_root);
	root.SetBool("ShowBoundingBoxes", show_bounding_boxes);

	ParsonArray componentArray = root.SetArray("Components");

	for (uint i = 0; i < components.size(); ++i)
	{
		ParsonNode componentNode = componentArray.SetNode(components[i]->GetNameFromType());
		components[i]->SaveState(componentNode);
	}

	return ret;
}

bool GameObject::LoadState(ParsonNode& root)
{
	bool ret = true;


	
	ForceUID((uint)root.GetNumber("UID"));
	parent_uid = (uint)root.GetNumber("ParentUID");

	prefabID = (uint)root.GetNumber("PrefabID");
	isPrefab = root.GetBool("IsPrefab");

	name				= root.GetString("Name");
	isActive			= root.GetBool("IsActive");
	isStatic			= root.GetBool("IsStatic");
	is_scene_root		= root.GetBool("IsSceneRoot");
	show_bounding_boxes = root.GetBool("ShowBoundingBoxes");

	// Recalculate AABB and OBB

	ParsonArray componentsArray = root.GetArray("Components");

	for (uint i = 0; i < componentsArray.size; ++i)
	{
		ParsonNode componentNode = componentsArray.GetNode(i);
		
		if (!componentNode.NodeIsValid())
			continue;
		
		ComponentType type	= (ComponentType)((int)componentNode.GetNumber("Type"));

		if (type < (ComponentType)0)
			continue;

		if (type == ComponentType::TRANSFORM)
		{
			GetComponent<C_Transform>()->LoadState(componentNode);
			continue;
		}
		else
		{
			Component* component = nullptr;

			switch (type)
			{
			//case COMPONENT_TYPE::TRANSFORM:		{ component = new C_Transform(this); }			break;
			case ComponentType::MESH:				{ component = new C_Mesh(this); }				break;
			case ComponentType::MATERIAL:			{ component = new C_Material(this); }			break;
			case ComponentType::LIGHT:				{ component = new C_Light(this); }				break;
			case ComponentType::CAMERA:				{ component = new C_Camera(this); } 			break;
			case ComponentType::ANIMATOR:			{ component = new C_Animator(this); }			break;
			case ComponentType::ANIMATION:			{ component = new C_Animation(this); }			break;
			case ComponentType::AUDIOSOURCE:		{ component = new C_AudioSource(this); }		break;
			case ComponentType::AUDIOLISTENER:		{ component = new C_AudioListener(this); }		break;
			case ComponentType::SCRIPT:				{ component = new C_Script(this); }				break;
			case ComponentType::RIGIDBODY:			{ component = new C_RigidBody(this); }			break;
			case ComponentType::BOX_COLLIDER:		{ component = new C_BoxCollider(this); }		break;
			case ComponentType::SPHERE_COLLIDER:	{ component = new C_SphereCollider(this); }		break;
			case ComponentType::CAPSULE_COLLIDER:	{ component = new C_CapsuleCollider(this); }	break;
			case ComponentType::PLAYER_CONTROLLER:	{ component = new C_PlayerController(this); }	break;
			case ComponentType::BULLET_BEHAVIOR:	{ component = new C_BulletBehavior(this); }		break;
			case ComponentType::PROP_BEHAVIOR:		{ component = new C_PropBehavior(this); }		break;
			case ComponentType::CAMERA_BEHAVIOR:	{ component = new C_CameraBehavior(this); }		break;
			case ComponentType::GATE_BEHAVIOR:		{ component = new C_GateBehavior(this); }		break;
			case ComponentType::PARTICLE_SYSTEM:	{ component = new C_ParticleSystem(this); }		break;
			case ComponentType::CANVAS:				{ component = new C_Canvas(this); }				break;
			case ComponentType::UI_IMAGE:			{ component = new C_UI_Image(this); }			break;
			case ComponentType::UI_TEXT:			{ component = new C_UI_Text(this); }			break;
			}

			if (component != nullptr)
			{
				if ((component->GetType() == ComponentType::CAMERA) && (App->gameState == GameState::PLAY)) //TODO fix this hardcode
					App->camera->SetCurrentCamera((C_Camera*)component);

				if (component->LoadState(componentNode))
					components.push_back(component);
				else 
				{
					component->CleanUp();
					delete component;
				}
			}
		}
	}

	return ret;
}

// --- GAMEOBJECT METHODS ---
void GameObject::FreeComponents()
{
	transform = nullptr;

	for (uint i = 0; i < components.size(); ++i)
	{
		components[i]->CleanUp();

		RELEASE(components[i]);
	}

	if(!components.empty()) components.clear();
}

void GameObject::FreeChilds()
{
	if (parent != nullptr)
	{
		parent->DeleteChild(this);												// Deleting this GameObject from the childs list of its parent.
	}

	/*if (parent != nullptr)													// Dirty fix to avoid innecessary calls to GetAllChilds().
	{
		std::vector<GameObject*> childsToDelete;
		GetAllChilds(childsToDelete);
		for (uint i = 0; i < childsToDelete.size(); ++i)
		{
			childsToDelete[i]->parent = nullptr;
			childsToDelete[i]->to_delete = true;
		}

		childsToDelete.clear();
	}*/

	for (uint i = 0; i < childs.size(); ++i)
	{	
		if (childs[i] != nullptr)
		{
			childs[i]->parent = nullptr;
			childs[i]->to_delete = true;									// Will set the children of the GameObject being deleted to be deleted too in M_Scene's game_objects vector.
			//childs[i]->CleanUp();											// Recursively cleaning up the the childs.
		}
	}

	childs.clear();
}

void GameObject::UpdateBoundingBoxes()
{
	std::vector<C_Mesh*> cMeshes;
	GetComponents<C_Mesh>(cMeshes);

	for (uint i = 0; i < cMeshes.size(); ++i)
	{
		if (cMeshes[i] == nullptr || cMeshes[i]->GetMesh() == nullptr)
		{
			continue;
		}
		
		obb = cMeshes[i]->GetMesh()->GetAABB();
		obb.Transform(GetComponent<C_Transform>()->GetWorldTransform());

		aabb.SetNegativeInfinity();
		aabb.Enclose(obb);
	}

	cMeshes.clear();
}

AABB GameObject::GetAABB() const
{
	return aabb;
}

float3* GameObject::GetAABBVertices() const
{
	return aabb_vertices;
}

void GameObject::GetRenderers(std::vector<MeshRenderer>& meshRenderers, std::vector<CuboidRenderer>& cuboidRenderers, std::vector<SkeletonRenderer>& skeletonRenderers)
{	
	/*if (to_delete || (parent != nullptr && parent->to_delete))			// TMP Quickfix. Deleted GameObjects could potentially generate Renderers. Fix the issue at the root later.
	{
		return;
	}*/
	
	std::vector<C_Mesh*> cMeshes;
	GetComponents<C_Mesh>(cMeshes);

	C_Transform* cTransform = GetComponent<C_Transform>();
	C_Material* cMaterial	= GetComponent<C_Material>();
	C_Camera* cCamera		= GetComponent<C_Camera>();
	C_Animator* cAnimation	= GetComponent<C_Animator>();

	for (uint i = 0; i < cMeshes.size(); ++i)
	{
		if (cMeshes[i] != nullptr)
		{
			if (cMeshes[i]->IsActive() && cMeshes[i]->GetMesh() != nullptr)
			{				
				meshRenderers.push_back(MeshRenderer(cTransform->GetWorldTransformPtr(), cMeshes[i], cMaterial));
			}
		}
	}

	cMeshes.clear();

	if (cCamera != nullptr)
	{
		if (!cCamera->FrustumIsHidden() && App->gameState != GameState::PLAY)
		{
			cuboidRenderers.push_back(CuboidRenderer(cCamera->GetFrustumVertices(), CuboidType::FRUSTUM));
		}
	}

	if (cAnimation != nullptr)
	{
		if (cAnimation->GetShowBones() || App->renderer->GetRenderSkeletons())
		{
			skeletonRenderers.push_back(SkeletonRenderer(cAnimation->GetDisplayBones()));
		}
	}

	if ((show_bounding_boxes || App->renderer->GetRenderBoundingBoxes()) && App->gameState != GameState::PLAY)
	{
		obb.GetCornerPoints(obb_vertices);
		aabb.GetCornerPoints(aabb_vertices);

		cuboidRenderers.push_back(CuboidRenderer(obb_vertices, CuboidType::OBB));
		cuboidRenderers.push_back(CuboidRenderer(aabb_vertices, CuboidType::AABB));
	}

	if (App->renderer->GetRenderColliders())
	{
		C_BoxCollider* collider = GetComponent<C_BoxCollider>();
		if (collider && collider->ToShowCollider())
		{
			cuboidRenderers.push_back(CuboidRenderer(collider->GetCornerPoints(), CuboidType::COLLIDER));
		}
	}
}

// --- PARENT/CHILDS METHODS
bool GameObject::SetParent(GameObject* newParent)
{
	bool success = true;
	
	if (newParent == nullptr)
	{
		LOG("[ERROR] Game Objects: SetParent() operation failed! Error: New parent was nullptr.");
		return false;
	}

	if (newParent->NewChildIsOwnParent(this))
	{
		LOG("[ERROR] Game Objects: Cannot re-parent parents into their own children!");
		return false;
	}
	
	if (parent != nullptr)
	{
		success = parent->DeleteChild(this);
		if (success)
		{
			GetComponent<C_Transform>()->SyncLocalToWorld();
		}
		else
		{
			LOG("[ERROR] Game Objects: DeleteChild() operation failed! Error: Child could not be found in Parent.");
			return false;
		}
	}

	success = newParent->AddChild(this);

	if (success)
	{
		parent = newParent;
	}
	else
	{
		LOG("[ERROR] GameObjects: AddChild() operation failed! Error: Check for AddChild() errors in the Console Log");
		
		if (parent != nullptr)
		{
			parent->AddChild(this);																						// Safety check to not lose the game object to the void.
		}																												// The GameObject will be reassigned as a child of the prev parent.
	}

	return success;
}

bool GameObject::AddChild(GameObject* child)
{
	bool ret = true;
	
	if (child->is_master_root)
	{
		LOG("[ERROR] Game Objects: AddChild() operation failed! Error: %s is the master root object!", child->name.c_str());
		return false;
	}
	
	if (!is_master_root && child->is_scene_root)
	{
		LOG("[ERROR] Game Objects: AddChild() operation failed! Error: %s is current scene root object!", child->name.c_str());
		return false;
	}
	
	if (!is_master_root && !is_scene_root)
	{
		if (NewChildIsOwnParent(child))
		{
			LOG("[ERROR] GameObjects: AddChild() operation failed! Error: Cannot re-parent parents into their own children!");
			return false;
		}
	}

	/*if (child->parent != nullptr)
	{	
		child->parent->DeleteChild(child);
		child->GetTransformComponent()->sync_local_to_global = true;
	}

	child->parent = this;*/

	childs.push_back(child);

	return ret;
}

bool GameObject::NewChildIsOwnParent(GameObject* child)
{
	bool ret = false;
	
	if (!child->HasChilds())												// If the child does not have any children, then it cannot be the parent of this object.
	{
		return false;
	}

	GameObject* parentItem = this->parent;									// Will set the parent of this object as the starting point of the search.
	
	while (parentItem != nullptr && !parentItem->is_scene_root)			// Iterate back up to the root object, as it is the parent of everything in the scene. (First check is TMP)
	{
		if (parentItem == child)											// Child is the parent of one of the parent objects of this object (the one which called AddChild())
		{
			ret = true;														// A parent of this object that had the passed child as the parent has been found.
			break;
		}

		parentItem = parentItem->parent;									// Setting the next parent GameObject to iterate.
	}

	// --- Adding a parent into a child
	/*for (uint i = 0; i < child->childs.size(); ++i)						// Iterating all the childs of the child.
	{
		child->parent->AddChild(child->childs[i]);							// Re-setting the parent of the childs to the parent of the passed child (root->GO->childs => root->childs->GO)
	}

	child->childs.clear();*/

	return ret;
}

bool GameObject::DeleteChild(GameObject* child)
{
	bool ret = false;
	
	for (uint i = 0; i < childs.size(); ++i)
	{
		if (childs[i] == child)
		{
			childs.erase(childs.begin() + i);
			ret = true;
			break;
		}
	}
	
	return ret;
}

bool GameObject::HasChilds() const
{
	return !childs.empty();
}

void GameObject::GetAllChilds(std::vector<GameObject*>& childs)
{
	if (this->childs.empty())
	{
		//LOG("[WARNING] Game Object: GameObject { %s } did not have any childs!", this->GetName());
		return;
	}
	
	for (uint i = 0; i < this->childs.size(); ++i)
	{
		childs.push_back(this->childs[i]);
		this->childs[i]->GetAllChilds(childs);
	}
}

void GameObject::GetAllChilds(std::map<std::string, GameObject*>& childs)
{
	if (this->childs.empty())
	{
		//LOG("[WARNING] Game Object: GameObject { %s } did not have any childs!", this->GetName());
		return;
	}

	for (uint i = 0; i < this->childs.size(); ++i)
	{
		childs.emplace(this->childs[i]->GetName(), this->childs[i]);
		this->childs[i]->GetAllChilds(childs);
	}
}

void GameObject::GetAllChilds(std::unordered_map<std::string, GameObject*>& childs)
{
	if (this->childs.empty())
	{
		LOG("[WARNING] Game Object: GameObject { %s } did not have any childs!");
		return;
	}

	for (uint i = 0; i < this->childs.size(); ++i)
	{
		childs.emplace(this->childs[i]->GetName(), this->childs[i]);
		this->childs[i]->GetAllChilds(childs);
	}
}

GameObject* GameObject::FindChild(const char* childName)
{	
	if (childName == nullptr)
	{
		LOG("[ERROR] Game Object: Could not Find Child in GameObject { %s }! Error: Argument const char* string was nullptr.");
		return nullptr;
	}
	
	/*std::vector<GameObject*> childs;
	GetAllChilds(childs);
	for (uint i = 0; i < childs.size(); ++i)
	{
		if (strcmp(childs[i]->GetName(), childName) == 0)
		{
			return childs[i];
		}
	}*/

	GameObject* child = nullptr;
	for (uint i = 0; i < childs.size(); ++i)
	{
		if (strcmp(childs[i]->GetName(), childName) == 0)
		{
			return childs[i];
		}

		child = childs[i]->FindChild(childName);
		if (child != nullptr)
		{
			return child;
		}
	}

	return nullptr;
}

void GameObject::SetAsPrefab(uint _prefabID)
{
	//set the childs to the prefab uid and the bool
	prefabID = _prefabID;
	isPrefab = true;

	for(auto child = childs.begin() ; child != childs.end();child++)
	{
		(*child)->SetAsPrefab(_prefabID);
	}
}

// --- GAME OBJECT GETTERS AND SETTERS ---
const char* GameObject::GetName() const
{
	return name.c_str();
}

bool GameObject::IsActive() const
{
	return isActive;
}

bool GameObject::IsStatic() const
{
	return isStatic;
}

void GameObject::ForceUID(const uint32& UID)
{
	uid = UID;
}

void GameObject::SetName(const char* newName)
{
	name = newName;
}

void GameObject::SetIsActive(const bool& setTo)
{
	isActive = setTo;

	SetChildsIsActive(setTo, this);
}

void GameObject::SetIsStatic(const bool& setTo)
{
	isStatic = setTo;

	SetChildsIsStatic(setTo, this);
}

void GameObject::SetChildsIsActive(const bool& setTo, GameObject* parent)
{
	if (parent != nullptr)
	{
		for (uint i = 0; i < parent->childs.size(); ++i)
		{
			parent->childs[i]->isActive = setTo;

			SetChildsIsActive(setTo, parent->childs[i]);
		}
	}
}

void GameObject::SetChildsIsStatic(const bool& setTo, GameObject* parent)
{
	if (parent != nullptr)
	{
		for (uint i = 0; i < parent->childs.size(); ++i)
		{
			parent->childs[i]->isStatic = setTo;

			SetChildsIsStatic(setTo, parent->childs[i]);
		}
	}
}

uint32 GameObject::GetParentUID() const
{
	return parent_uid;
}

void GameObject::SetParentUID(const uint32& parentUID)
{
	parent_uid = parentUID;
}

// --- COMPONENT METHODS ---
Component* GameObject::CreateComponent(ComponentType type)
{
	Component* component = nullptr;

	if (type == ComponentType::TRANSFORM && GetComponent<C_Transform>() != nullptr)
	{
		LOG("[ERROR] Transform Component could not be added to %s! Error: No duplicates allowed!", name.c_str());
		return nullptr;
	}
	if (type == ComponentType::MATERIAL && GetComponent<C_Material>() != nullptr)
	{
		LOG("[ERROR] Material Component could not be added to %s! Error: No duplicates allowed!", name.c_str());
		return nullptr;
	}
	if (type == ComponentType::RIGIDBODY && GetComponent<C_RigidBody>() != nullptr)
	{
		LOG("[ERROR] RigidBody Component could not be added to %s! Error: No duplicates allowed!", name.c_str());
		return nullptr;
	}
	if (type == ComponentType::PLAYER_CONTROLLER && GetComponent<C_PlayerController>() != nullptr)
	{
		LOG("[ERROR] Player Controller Component could not be added to %s! Error: No duplicates allowed!", name.c_str());
		return nullptr;
	}
	if (type == ComponentType::UI_IMAGE && GetComponent<C_UI_Image>() != nullptr)
	{
		LOG("[ERROR] Player Controller Component could not be added to %s! Error: No duplicates allowed!", name.c_str());
		return nullptr;
	}

#ifndef GAMEBUILD
	//TODO: Maybe this is avoidable !!!!!!!!
	/*std::vector<C_Script*>scripts;
	if (type == ComponentType::SCRIPT && GetComponents<C_Script>(scripts)) 
	{
		for(int i = 0; i<scripts.size();++i)
		{
			if (!(scripts[i]->resource != nullptr)) 
			{
				LOG("[ERROR] Script Component could not be added to %s! Error: Empty Script to fill already exists!", name.c_str());
				return nullptr;
			}
		}
	}*/
#endif

	switch(type)
	{
	case ComponentType::TRANSFORM:			{ component = new C_Transform(this); }			break;
	case ComponentType::MESH:				{ component = new C_Mesh(this); }				break;
	case ComponentType::MATERIAL:			{ component = new C_Material(this); }			break;
	case ComponentType::LIGHT:				{ component = new C_Light(this); }				break;
	case ComponentType::CAMERA:				{ component = new C_Camera(this); }				break;
	case ComponentType::ANIMATOR:			{ component = new C_Animator(this); }			break;
	case ComponentType::ANIMATION:			{ component = new C_Animation(this); }			break;
	case ComponentType::AUDIOSOURCE:		{ component = new C_AudioSource(this); }		break;
	case ComponentType::AUDIOLISTENER:		{ component = new C_AudioListener(this); }		break;
	case ComponentType::RIGIDBODY:			{ component = new C_RigidBody(this); }			break;
	case ComponentType::BOX_COLLIDER:		{ component = new C_BoxCollider(this); }		break;
	case ComponentType::SPHERE_COLLIDER:	{ component = new C_SphereCollider(this); }		break;
	case ComponentType::CAPSULE_COLLIDER:	{ component = new C_CapsuleCollider(this); }	break;
	case ComponentType::PARTICLE_SYSTEM: 	{ component = new C_ParticleSystem(this); }		break;
	case ComponentType::CANVAS:				{ component = new C_Canvas(this); }				break;
	case ComponentType::UI_IMAGE:			{ component = new C_UI_Image(this); }			break;
	case ComponentType::UI_TEXT:			{ component = new C_UI_Text(this); }			break;
	case ComponentType::SCRIPT:				{ component = new C_Script(this); }				break;
	case ComponentType::PLAYER_CONTROLLER:	{ component = new C_PlayerController(this); }	break;
	case ComponentType::BULLET_BEHAVIOR:	{ component = new C_BulletBehavior(this); }		break;
	case ComponentType::PROP_BEHAVIOR:		{ component = new C_PropBehavior(this); }		break;
	case ComponentType::CAMERA_BEHAVIOR:	{ component = new C_CameraBehavior(this); }		break;
	case ComponentType::GATE_BEHAVIOR:		{ component = new C_GateBehavior(this); }		break;
	}

	if (component != nullptr)
		components.push_back(component);

	return component;
}

bool GameObject::DeleteComponent(Component* componentToDelete)
{
	switch (componentToDelete->GetType())
	{
	case ComponentType::MESH: 
		App->renderer->DeleteFromMeshRenderers((C_Mesh*)componentToDelete); 
		show_bounding_boxes = false;
		break;
	}

	std::string componentName = componentToDelete->GetNameFromType();
	
	if (componentToDelete != nullptr)
	{
		for (uint i = 0; i < components.size(); ++i)
		{
			if (components[i] == componentToDelete)
			{
				components[i]->CleanUp();
				
				RELEASE(components[i]);

				components.erase(components.begin() + i);

				return true;
			}
		}
	}

	LOG("[STATUS] Deleted Component %s of Game Object %s", componentName.c_str(), name.c_str());

	return false;
}

const std::vector<Component*>& GameObject::GetAllComponents() const
{
	return components;
}

bool GameObject::GetAllComponents(std::vector<Component*>& components) const
{
	for (uint i = 0; i < this->components.size(); ++i)
	{
		components.push_back(this->components[i]);
	}

	return components.empty() ? false : true;
}

// ---
uint32 GameObject::GetUID() const
{
	return uid;
}