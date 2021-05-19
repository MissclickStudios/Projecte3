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
#include "C_ParticleSystem.h"
#include "C_NavMeshAgent.h"
//#include "C_PlayerController.h"
//#include "C_BulletBehavior.h"
//#include "C_PropBehavior.h"
//#include "C_CameraBehavior.h"
//#include "C_GateBehavior.h"

#include "C_Canvas.h"
#include "C_UI.h"
#include "C_UI_Image.h"
#include "C_UI_Text.h"
#include "C_UI_Button.h"

#include "C_2DAnimator.h"

#include "GameObject.h"

#include "MemoryManager.h"

GameObject::GameObject() :
uid						(Random::LCG::GetRandomUint()),
parent_uid				(0),
name					("GameObject"),
isActive				(true),
isStatic				(false),
parent					(nullptr),
transform				(nullptr),
isMasterRoot			(false),
isSceneRoot				(false),
isBone					(false),
maintainThroughScenes	(false),
toDelete				(false),
show_bounding_boxes		(false)
{
	transform = (C_Transform*)CreateComponent(ComponentType::TRANSFORM);

	obb.SetNegativeInfinity();
	aabb.SetNegativeInfinity();

	obb_vertices	= new float3[8];																		// Bounding boxes will always have 8 vertices as they are Cuboids.
	aabb_vertices	= new float3[8];																		// Bounding boxes will always have 8 vertices as they are Cuboids.
}

GameObject::GameObject(std::string name, bool isActive, bool isStatic) :
uid						(Random::LCG::GetRandomUint()),
parent_uid				(0),
name					(name),
isActive				(isActive),
isStatic				(isStatic),
parent					(nullptr),
transform				(nullptr),
isMasterRoot			(false),
isSceneRoot				(false),
isBone					(false),
maintainThroughScenes	(false),
toDelete				(false),
show_bounding_boxes		(false)
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

bool GameObject::Start()
{
	for (auto component = components.cbegin(); component != components.end(); ++component)
	{
		if ((*component)->IsActive())
		{
			(*component)->Start();
		}
	}
	
	return true;
}

bool GameObject::Update()
{
	OPTICK_CATEGORY("Game Object Update",Optick::Category::Update);
	bool ret = true;

	for (uint i = 0; i < components.size(); ++i)
	{
		if (components[i]->IsActive())
		{
			components[i]->Update();
		}
	}

	//UpdateBoundingBoxes();																					// Make the call in C_Transform after receiving a dirty flag?

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
	root.SetNumber("UID", uid);

	uint parentUID = (parent != nullptr) ? parent->uid : 0;
	root.SetNumber("ParentUID", parentUID);

	root.SetNumber("PrefabID", prefabID);
	root.SetBool("IsPrefab", isPrefab);

	root.SetString("Name", name.c_str());
	root.SetBool("IsActive", isActive);
	root.SetBool("IsStatic", isStatic);
	root.SetBool("IsSceneRoot", isSceneRoot);
	root.SetBool("MaintainThroughScenes", maintainThroughScenes);
	root.SetBool("ShowBoundingBoxes", show_bounding_boxes);
	ParsonNode navNode = root.SetNode("NavigationInfo");
	navNode.SetBool("isNavigable",isNavigable);
	navNode.SetInteger("navigationArea", navigationArea);

	ParsonArray componentArray = root.SetArray("Components");

	for (uint i = 0; i < components.size(); ++i)
	{
		ParsonNode componentNode = componentArray.SetNode(components[i]->GetNameFromType());
		components[i]->SaveState(componentNode);
	}

	return true;
}

bool GameObject::LoadState(ParsonNode& root)
{
	ForceUID((uint)root.GetNumber("UID"));
	parent_uid	= (uint)root.GetNumber("ParentUID");

	prefabID	= (uint)root.GetNumber("PrefabID");
	isPrefab	= root.GetBool("IsPrefab");

	name					= root.GetString("Name");
	isActive				= root.GetBool("IsActive");
	isStatic				= root.GetBool("IsStatic");
	isSceneRoot				= root.GetBool("IsSceneRoot");
	maintainThroughScenes	= root.GetBool("MaintainThroughScenes");
	show_bounding_boxes		= root.GetBool("ShowBoundingBoxes");

	ParsonNode navNode = root.GetNode("NavigationInfo");
	if (navNode.NodeIsValid()) 
	{
		isNavigable = navNode.GetBool("isNavigable");
		navigationArea = navNode.GetInteger("navigationArea");
	}

	ParsonArray componentsArray = root.GetArray("Components");
	for (uint i = 0; i < componentsArray.size; ++i)
	{
		ParsonNode componentNode = componentsArray.GetNode(i);
		if (!componentNode.NodeIsValid())
		{
			continue;
		}
		
		ComponentType type	= (ComponentType)((int)componentNode.GetNumber("Type"));
		if (type == ComponentType::NONE)
		{
			continue;
		}
		if (type == ComponentType::TRANSFORM)
		{
			transform->LoadState(componentNode);
			continue;
		}

		Component* component = CreateComponent(type, false);
		if (component != nullptr)
		{
			if (component->LoadState(componentNode))
			{
				component->Start();
				components.push_back(component);

				if (component->GetType() == ComponentType::CAMERA && App->gameState == GameState::PLAY)
				{
					App->camera->SetCurrentCamera((C_Camera*)component);
				}
			}
			else 
			{
				LOG("[WARNING] Game Object: Could not Load State of Component { %s } of Game Object { %s }!", component->GetNameFromType(), name.c_str());

				component->CleanUp();
				delete component;
			}
		}
	}

	return true;
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
			childs[i]->toDelete = true;											// Will set the children of the GameObject being deleted to be deleted too in M_Scene's game_objects vector.

			/*if (!childs[i]->maintainThroughScenes)								// Dirty fix on issue generated at CleanUpCurrentScene() in M_Scene().
			{
				childs[i]->toDelete = true;										// Will set the children of the GameObject being deleted to be deleted too in M_Scene's game_objects vector.
				//childs[i]->CleanUp();											// Recursively cleaning up the the childs.
			}*/
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
	OPTICK_CATEGORY("Game Object: Get Renderers", Optick::Category::GameLogic);
	
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
				meshRenderers.push_back(MeshRenderer(cTransform, cMeshes[i], cMaterial));
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
		UpdateBoundingBoxes();																						// Make the call in C_Transform after receiving a dirty flag?

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
	
	if (child->isMasterRoot)
	{
		LOG("[ERROR] Game Objects: AddChild() operation failed! Error: %s is the master root object!", child->name.c_str());
		return false;
	}
	
	if (!isMasterRoot && child->isSceneRoot)
	{
		LOG("[ERROR] Game Objects: AddChild() operation failed! Error: %s is current scene root object!", child->name.c_str());
		return false;
	}
	
	if (!isMasterRoot && !isSceneRoot)
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
	
	while (parentItem != nullptr && !parentItem->isSceneRoot)			// Iterate back up to the root object, as it is the parent of everything in the scene. (First check is TMP)
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

void GameObject::GetAllChilds(std::vector<GameObject*>& children)
{
	if (childs.empty())
	{
		//LOG("[WARNING] Game Object: GameObject { %s } did not have any childs!", this->GetName());
		return;
	}
	
	for (uint i = 0; i < childs.size(); ++i)
	{
		children.push_back(childs[i]);
		childs[i]->GetAllChilds(children);
	}
}

void GameObject::GetAllChilds(std::map<std::string, GameObject*>& children)
{
	if (childs.empty())
	{
		//LOG("[WARNING] Game Object: GameObject { %s } did not have any childs!", this->GetName());
		return;
	}

	for (uint i = 0; i < childs.size(); ++i)
	{
		children.emplace(childs[i]->GetName(), childs[i]);
		childs[i]->GetAllChilds(children);
	}
}

void GameObject::GetAllChilds(std::unordered_map<std::string, GameObject*>& children)
{
	if (childs.empty())
	{
		LOG("[WARNING] Game Object: GameObject { %s } did not have any childs!");
		return;
	}

	for (uint i = 0; i < this->childs.size(); ++i)
	{
		children.emplace(childs[i]->GetName(), childs[i]);
		childs[i]->GetAllChilds(children);
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

void GameObject::GetAllParents(std::vector<GameObject*>& parents)
{
	if (parent->isSceneRoot)
	{
		return;
	}
	
	parents.push_back(parent);
	parent->GetAllParents(parents);
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
	if (isBone)
	{
		LOG("[WARNING] Game Object: Game Objects that are Animation Bones cannot be renamed!");
		return;
	}
	
	name = newName;
}

void GameObject::SetIsActive(const bool setTo)
{
	isActive = setTo;

	SetChildsIsActive(setTo, this);
}

void GameObject::SetIsStatic(const bool setTo)
{
	isStatic = setTo;

	SetChildsIsStatic(setTo, this);
}

void GameObject::SetChildsIsActive(const bool setTo, GameObject* parent)
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

void GameObject::SetChildsIsStatic(const bool setTo, GameObject* parent)
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

bool GameObject::GetMaintainThroughScenes() const
{
	return maintainThroughScenes;
}

void GameObject::SetMaintainThroughScenes(const bool setTo)
{
	maintainThroughScenes = setTo;

	std::vector<GameObject*> childs;
	std::vector<GameObject*> parents;

	GetAllChilds(childs);
	GetAllParents(parents);

	for (auto child = childs.cbegin(); child != childs.cend(); ++child)
	{
		(*child)->maintainThroughScenes = setTo;
	}
	for (auto parent = parents.cbegin(); parent != parents.cend(); ++parent)
	{
		(*parent)->maintainThroughScenes = setTo;
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
Component* GameObject::CreateComponent(ComponentType type, bool addComponent)
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
	//if (type == ComponentType::PLAYER_CONTROLLER && GetComponent<C_PlayerController>() != nullptr)
	//{
	//	LOG("[ERROR] Player Controller Component could not be added to %s! Error: No duplicates allowed!", name.c_str());
	//	return nullptr;
	//}
	if (type == ComponentType::UI_IMAGE && GetComponent<C_UI_Image>() != nullptr)
	{
		LOG("[ERROR] Player Controller Component could not be added to %s! Error: No duplicates allowed!", name.c_str());
		return nullptr;
	}

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
	case ComponentType::UI_BUTTON:			{ component = new C_UI_Button(this); }			break;
	//case ComponentType::PLAYER_CONTROLLER:	{ component = new C_PlayerController(this); }	break;
	//case ComponentType::BULLET_BEHAVIOR:	{ component = new C_BulletBehavior(this); }		break;
	//case ComponentType::PROP_BEHAVIOR:		{ component = new C_PropBehavior(this); }		break;
	//case ComponentType::CAMERA_BEHAVIOR:	{ component = new C_CameraBehavior(this); }		break;
	//case ComponentType::GATE_BEHAVIOR:		{ component = new C_GateBehavior(this); }		break;
	case ComponentType::ANIMATOR2D:			{ component = new C_2DAnimator(this); }			break;
	case ComponentType::NAVMESH_AGENT:		{ component = new C_NavMeshAgent(this); }		break;
	}

	if (component != nullptr && addComponent)
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

void GameObject::ReplaceComponent(Component* newComponent)
{
	for (auto comp = components.begin(); comp != components.end(); comp++)
	{
		if (newComponent->GetType() == (*comp)->GetType())
		{
			DeleteComponent((*comp));

			Component* a = (C_Transform*)CreateComponent(newComponent->GetType());

			*a = *(C_Transform*)newComponent;
		}
	}
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

void* GameObject::GetScript(const char* scriptName)
{
	for (uint i = 0; i < components.size(); ++i) 
		if (components[i]->GetType() == ComponentType::SCRIPT) 
			if (scriptName == ((C_Script*)components[i])->GetDataName())
				return ((C_Script*)components[i])->GetScriptData();
			
	return nullptr;
}

void GameObject::GetUiComponents(std::vector<C_UI*>& uiComponents)
{
	for (uint i = 0; i < components.size(); ++i)
	{																					
		if (components[i] != nullptr && (components[i]->GetType() == ComponentType::UI_BUTTON || components[i]->GetType() == ComponentType::UI_IMAGE || components[i]->GetType() == ComponentType::UI_TEXT))
			uiComponents.push_back((C_UI*)components[i]);						
	}																						
}

C_UI* GameObject::GetUiComponent()
{
	for (uint i = 0; i < components.size(); ++i)
	{
		if (components[i] != nullptr && (components[i]->GetType() == ComponentType::UI_BUTTON || components[i]->GetType() == ComponentType::UI_IMAGE || components[i]->GetType() == ComponentType::UI_TEXT))
			return (C_UI*)components[i];
	}
	return nullptr;
}

void GameObject::SetUiChildOrder(int index)
{
	for (uint i = 0; i < components.size(); ++i)
	{
		if (components[i] != nullptr && (components[i]->GetType() == ComponentType::UI_BUTTON || components[i]->GetType() == ComponentType::UI_IMAGE || components[i]->GetType() == ComponentType::UI_TEXT))
			((C_UI*)components[i])->childOrder = index;
	}
}

// ---
uint32 GameObject::GetUID() const
{
	return uid;
}