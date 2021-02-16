#include "VariableTypedefs.h"
#include "Macros.h"
#include "Log.h"

#include "JSONParser.h"
#include "Random.h"

#include "Application.h"
#include "M_Renderer3D.h"																				// TMP. Move the Renderers generation elsewhere.

#include "R_Mesh.h"

#include "Component.h"
#include "C_Transform.h"
#include "C_Mesh.h"
#include "C_Material.h"
#include "C_Light.h"
#include "C_Camera.h"
#include "C_Animator.h"
#include "C_Animation.h"

#include "GameObject.h"

#include "MemoryManager.h"

GameObject::GameObject() :
uid					(Random::LCG::GetRandomUint()),
parent_uid			(0),
name				("GameObject"),
is_active			(true),
is_static			(false),
parent				(nullptr),
transform			(nullptr),
is_master_root		(false),
is_scene_root		(false),
is_bone				(false),
to_delete			(false),
show_bounding_boxes	(false)
{
	transform	= (C_Transform*)CreateComponent(COMPONENT_TYPE::TRANSFORM);

	obb.SetNegativeInfinity();
	aabb.SetNegativeInfinity();

	obb_vertices	= new float3[8];																	// Bounding boxes will always have 8 vertices as they are Cuboids.
	aabb_vertices	= new float3[8];																	// Bounding boxes will always have 8 vertices as they are Cuboids.
}

GameObject::GameObject(std::string name, bool is_active, bool is_static) :
uid					(Random::LCG::GetRandomUint()),
parent_uid			(0),
name				(name),
is_active			(is_active),
is_static			(is_static),
parent				(nullptr),
transform			(nullptr),
is_master_root		(false),
is_scene_root		(false),
is_bone				(false),
to_delete			(false),
show_bounding_boxes	(false)
{
	if (name.empty())
	{
		name = "GameObject";
	}

	transform = (C_Transform*)CreateComponent(COMPONENT_TYPE::TRANSFORM);

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

	uint parent_UID = (parent != nullptr) ? parent->uid : 0;
	root.SetNumber("ParentUID", parent_UID);

	root.SetString("Name", name.c_str());
	root.SetBool("IsActive", is_active);
	root.SetBool("IsStatic", is_static);
	root.SetBool("IsSceneRoot", is_scene_root);
	root.SetBool("ShowBoundingBoxes", show_bounding_boxes);

	ParsonArray component_array = root.SetArray("Components");

	for (uint i = 0; i < components.size(); ++i)
	{
		ParsonNode component_node = component_array.SetNode(components[i]->GetNameFromType());
		components[i]->SaveState(component_node);
	}

	return ret;
}

bool GameObject::LoadState(ParsonNode& root)
{
	bool ret = true;
	
	ForceUID((uint)root.GetNumber("UID"));
	parent_uid = (uint)root.GetNumber("ParentUID");

	name					= root.GetString("Name");
	is_active				= root.GetBool("IsActive");
	is_static				= root.GetBool("IsStatic");
	is_scene_root			= root.GetBool("IsSceneRoot");
	show_bounding_boxes		= root.GetBool("ShowBoundingBoxes");

	// Recalculate AABB and OBB

	ParsonArray components_array = root.GetArray("Components");

	for (uint i = 0; i < components_array.size; ++i)
	{
		ParsonNode component_node	= components_array.GetNode(i);
		
		if (!component_node.NodeIsValid())
		{
			continue;
		}
		
		COMPONENT_TYPE type	= (COMPONENT_TYPE)component_node.GetNumber("Type");

		if (type == COMPONENT_TYPE::TRANSFORM)
		{
			GetComponent<C_Transform>()->LoadState(component_node);
			continue;
		}
		else
		{
			Component* component = nullptr;

			switch (type)
			{
			//case COMPONENT_TYPE::TRANSFORM: { component = new C_Transform(this); }	break;
			case COMPONENT_TYPE::MESH:		{ component = new C_Mesh(this); }		break;
			case COMPONENT_TYPE::MATERIAL:	{ component = new C_Material(this); }	break;
			case COMPONENT_TYPE::LIGHT:		{ component = new C_Light(this); }		break;
			case COMPONENT_TYPE::CAMERA:	{ component = new C_Camera(this); }		break;
			case COMPONENT_TYPE::ANIMATOR:	{ component = new C_Animator(this); }	break;
			case COMPONENT_TYPE::ANIMATION: { component = new C_Animation(this); }	break;
			}

			if (component != nullptr)
			{
				component->LoadState(component_node);
				components.push_back(component);
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

	components.clear();
}

void GameObject::FreeChilds()
{
	if (parent != nullptr)
	{
		parent->DeleteChild(this);											// Deleting this GameObject from the childs list of its parent.
	}

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
	std::vector<C_Mesh*> c_meshes;
	GetComponents<C_Mesh>(c_meshes);

	for (uint i = 0; i < c_meshes.size(); ++i)
	{
		if (c_meshes[i] == nullptr || c_meshes[i]->GetMesh() == nullptr)
		{
			continue;
		}
		
		obb = c_meshes[i]->GetMesh()->GetAABB();
		obb.Transform(GetComponent<C_Transform>()->GetWorldTransform());

		aabb.SetNegativeInfinity();
		aabb.Enclose(obb);
	}

	c_meshes.clear();
}

AABB GameObject::GetAABB() const
{
	return aabb;
}

float3* GameObject::GetAABBVertices() const
{
	return aabb_vertices;
}

void GameObject::GetRenderers(std::vector<MeshRenderer>& mesh_renderers, std::vector<CuboidRenderer>& cuboid_renderers, std::vector<SkeletonRenderer>& skeleton_renderers)
{
	std::vector<C_Mesh*> c_meshes;
	GetComponents<C_Mesh>(c_meshes);

	C_Material* c_material		= GetComponent<C_Material>();
	C_Camera* c_camera			= GetComponent<C_Camera>();
	C_Animator* c_animation	= GetComponent<C_Animator>();

	for (uint i = 0; i < c_meshes.size(); ++i)
	{
		if (c_meshes[i] != nullptr)
		{
			if (c_meshes[i]->IsActive() && c_meshes[i]->GetMesh() != nullptr)
			{
				mesh_renderers.push_back(MeshRenderer(GetComponent<C_Transform>()->GetWorldTransform(), c_meshes[i], c_material));
			}
		}
	}

	c_meshes.clear();

	if (c_camera != nullptr)
	{
		if (!c_camera->FrustumIsHidden())
		{
			cuboid_renderers.push_back(CuboidRenderer(c_camera->GetFrustumVertices(), CUBOID_TYPE::FRUSTUM));
		}
	}

	if (c_animation != nullptr)
	{
		if (c_animation->GetShowBones() || App->renderer->GetRenderSkeletons())
		{
			skeleton_renderers.push_back(SkeletonRenderer(c_animation->GetDisplayBones()));
		}
	}

	if (show_bounding_boxes || App->renderer->GetRenderBoundingBoxes())
	{
		obb.GetCornerPoints(obb_vertices);
		aabb.GetCornerPoints(aabb_vertices);

		cuboid_renderers.push_back(CuboidRenderer(obb_vertices, CUBOID_TYPE::OBB));
		cuboid_renderers.push_back(CuboidRenderer(aabb_vertices, CUBOID_TYPE::AABB));
	}
}

// --- PARENT/CHILDS METHODS
bool GameObject::SetParent(GameObject* new_parent)
{
	bool success = true;
	
	if (new_parent == nullptr)
	{
		LOG("[ERROR] Game Objects: SetParent() operation failed! Error: New parent was nullptr.");
		return false;
	}

	if (new_parent->NewChildIsOwnParent(this))
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

	success = new_parent->AddChild(this);
	if (success)
	{
		parent = new_parent;
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

	GameObject* parent_item = this->parent;									// Will set the parent of this object as the starting point of the search.
	
	while (parent_item != nullptr && !parent_item->is_scene_root)			// Iterate back up to the root object, as it is the parent of everything in the scene. (First check is TMP)
	{
		if (parent_item == child)											// Child is the parent of one of the parent objects of this object (the one which called AddChild())
		{
			ret = true;														// A parent of this object that had the passed child as the parent has been found.
			break;
		}

		parent_item = parent_item->parent;									// Setting the next parent GameObject to iterate.
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
		LOG("[WARNING] Game Object: GameObject { %s } did not have any childs!");
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
		LOG("[WARNING] Game Object: GameObject { %s } did not have any childs!", this->GetName());
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

GameObject* GameObject::FindChild(const char* child_name)
{	
	if (child_name == nullptr)
	{
		LOG("[ERROR] Game Object: Could not Find Child in GameObject { %s }! Error: Argument const char* string was nullptr.");
		return nullptr;
	}
	
	/*std::vector<GameObject*> childs;
	GetAllChilds(childs);
	for (uint i = 0; i < childs.size(); ++i)
	{
		if (strcmp(childs[i]->GetName(), child_name) == 0)
		{
			return childs[i];
		}
	}*/

	GameObject* child = nullptr;
	for (uint i = 0; i < childs.size(); ++i)
	{
		if (strcmp(childs[i]->GetName(), child_name) == 0)
		{
			return childs[i];
		}

		child = childs[i]->FindChild(child_name);
		if (child != nullptr)
		{
			return child;
		}
	}

	return nullptr;
}

// --- GAME OBJECT GETTERS AND SETTERS ---
const char* GameObject::GetName() const
{
	return name.c_str();
}

bool GameObject::IsActive() const
{
	return is_active;
}

bool GameObject::IsStatic() const
{
	return is_static;
}

void GameObject::ForceUID(const uint32& UID)
{
	uid = UID;
}

void GameObject::SetName(const char* new_name)
{
	name = new_name;
}

void GameObject::SetIsActive(const bool& set_to)
{
	is_active = set_to;

	SetChildsIsActive(set_to, this);
}

void GameObject::SetIsStatic(const bool& set_to)
{
	is_static = set_to;

	SetChildsIsStatic(set_to, this);
}

void GameObject::SetChildsIsActive(const bool& set_to, GameObject* parent)
{
	if (parent != nullptr)
	{
		for (uint i = 0; i < parent->childs.size(); ++i)
		{
			parent->childs[i]->is_active = set_to;

			SetChildsIsActive(set_to, parent->childs[i]);
		}
	}
}

void GameObject::SetChildsIsStatic(const bool& set_to, GameObject* parent)
{
	if (parent != nullptr)
	{
		for (uint i = 0; i < parent->childs.size(); ++i)
		{
			parent->childs[i]->is_static = set_to;

			SetChildsIsStatic(set_to, parent->childs[i]);
		}
	}
}

uint32 GameObject::GetParentUID() const
{
	return parent_uid;
}

void GameObject::SetParentUID(const uint32& parent_UID)
{
	parent_uid = parent_UID;
}

// --- COMPONENT METHODS ---
Component* GameObject::CreateComponent(COMPONENT_TYPE type)
{
	Component* component = nullptr;

	if (type == COMPONENT_TYPE::TRANSFORM && GetComponent<C_Transform>() != nullptr)
	{
		LOG("[ERROR] Transform Component could not be added to %s! Error: No duplicates allowed!", name.c_str());
		return nullptr;
	}
	if (type == COMPONENT_TYPE::MATERIAL && GetComponent<C_Material>() != nullptr)
	{
		LOG("[ERROR] Material Component could not be added to %s! Error: No duplicates allowed!", name.c_str());
		return nullptr;
	}

	switch(type)
	{
	case COMPONENT_TYPE::TRANSFORM:	{ component = new C_Transform(this); }	break;
	case COMPONENT_TYPE::MESH:		{ component = new C_Mesh(this); }		break;
	case COMPONENT_TYPE::MATERIAL:	{ component = new C_Material(this); }	break;
	case COMPONENT_TYPE::LIGHT:		{ component = new C_Light(this); }		break;
	case COMPONENT_TYPE::CAMERA:	{ component = new C_Camera(this); }		break;
	case COMPONENT_TYPE::ANIMATOR:	{ component = new C_Animator(this); }	break;
	case COMPONENT_TYPE::ANIMATION: { component = new C_Animation(this); }	break;
	}

	if (component != nullptr)
	{
		components.push_back(component);
	}

	return component;
}

bool GameObject::DeleteComponent(Component* component_to_delete)
{
	switch (component_to_delete->GetType())
	{
	case COMPONENT_TYPE::MESH: 
		App->renderer->DeleteFromMeshRenderers((C_Mesh*)component_to_delete); 
		show_bounding_boxes = false;
		break;
	}

	std::string component_name = component_to_delete->GetNameFromType();
	
	if (component_to_delete != nullptr)
	{
		for (uint i = 0; i < components.size(); ++i)
		{
			if (components[i] == component_to_delete)
			{
				components[i]->CleanUp();
				
				RELEASE(components[i]);

				components.erase(components.begin() + i);

				return true;
			}
		}
	}

	LOG("[STATUS] Deleted Component %s of Game Object %s", component_name.c_str(), name.c_str());

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

uint32 GameObject::GetUID() const
{
	return uid;
}