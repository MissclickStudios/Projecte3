#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

#include "Macros.h"
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include "MathGeoBoundingBox.h"
#include "Component.h"

#include "C_Camera.h" //TODO: Why do we need to include this ????


class ParsonNode;
class C_Transform;

struct MeshRenderer;
struct CuboidRenderer;
struct SkeletonRenderer;
struct Light;

typedef unsigned __int32 uint32;
typedef unsigned int uint;

class NULL_API GameObject
{
public:
	GameObject();
	GameObject(std::string name , bool isActive = true, bool isStatic = false);
	~GameObject();

	bool			Update				();
	bool			CleanUp				();

	bool			SaveState			(ParsonNode& root) const;
	bool			LoadState			(ParsonNode& root);

public:
	void			FreeComponents						();
	void			FreeChilds							();
	
	void			UpdateBoundingBoxes					();
	AABB			GetAABB								() const;
	float3*			GetAABBVertices						() const;
	void			GetRenderers						(std::vector<MeshRenderer>& meshRenderers, std::vector<CuboidRenderer>& cuboidRenderers, 
															std::vector<SkeletonRenderer>& skeletonRenderers); // TODO: Get them elsewhere. Scene maybe?

public:																									// --- PARENT/CHILDS METHODS
	bool			SetParent							(GameObject* newParent);

	bool			AddChild							(GameObject* child);							// Adds the given child to this GO's childs. Also dels. it from prev. parent's childs.
	bool			NewChildIsOwnParent					(GameObject* child);							// Rets. true if passed child is being added to one of it's children or ch. of chs.
	bool			DeleteChild							(GameObject* child);							// Deletes the given child from childs. Returns false upon not finding the child.
	bool			HasChilds							() const;

	void			GetAllChilds						(std::vector<GameObject*>& childs);	
	void			GetAllChilds						(std::map<std::string, GameObject*>& childs);
	void			GetAllChilds						(std::unordered_map<std::string, GameObject*>& childs);
	GameObject*		FindChild							(const char* childName);

	void SetAsPrefab(uint _prefabID);	//Prefabs


public:																									// --- GAME OBJECT GETTERS AND SETTERS
	uint32			GetUID								() const;										//
	const char*		GetName								() const;										// 
	bool			IsActive							() const;										// 
	bool			IsStatic							() const;										// 

	void			ForceUID							(const uint32& UID);							// 
	void			SetName								(const char* newName);							// 
	void			SetIsActive							(const bool& setTo);							// 
	void			SetIsStatic							(const bool& setTo);							// 
	void			SetChildsIsActive					(const bool& setTo, GameObject* parent);		// 
	void			SetChildsIsStatic					(const bool& setTo, GameObject* parent);		// 

	uint32			GetParentUID						() const;
	void			SetParentUID						(const uint32& parentUID);

public:																									// --- COMPONENT GETTERS AND SETTERS
	Component*		CreateComponent						(ComponentType type);							// Creates a component of the given type and adds it to the components vector.
	bool			DeleteComponent						(Component* componentToDelete);					// Deletes the given component from the Components vector. Returs False on ERROR.
	

	const std::vector<Component*>&	GetAllComponents	() const;										// 
	bool							GetAllComponents	(std::vector<Component*>& components) const;	// 

	template<typename T>
	T* GetComponent() const
	{
		ComponentType type = T::GetType();

		if (type == ComponentType::TRANSFORM)															// Optimization to avoid having to look for the transform component each time.
		{
			return (T*)transform;																		// This can be applied as there will be ONLY ONE TRANSFORM COMPONENT PER GO.
		}

		for (int i = 0; i < components.size(); ++i)
		{
			if(components[i])
				if (components[i]->GetType() == type)
				{
					return (T*)components[i];
				}
		}

		return nullptr;
	}

	template <typename T>
	const char* GetComponentName() const
	{	
		ComponentType type = T::GetType();
		switch (type)
		{
		case ComponentType::TRANSFORM:			{ return "Transform"; }			break;
		case ComponentType::MESH:				{ return "Mesh"; }				break;
		case ComponentType::MATERIAL:			{ return "Material"; }			break;
		case ComponentType::LIGHT:				{ return "Light"; }				break;
		case ComponentType::CAMERA:				{ return "Camera"; }			break;
		case ComponentType::ANIMATOR:			{ return "Animator"; }			break;
		case ComponentType::ANIMATION:			{ return "Animation"; }			break;
		case ComponentType::AUDIOSOURCE:		{ return "Audio Source"; }		break;
		case ComponentType::AUDIOLISTENER:		{ return "Audio Listener"; }	break;
		case ComponentType::RIGIDBODY:			{ return "RigidBody"; }			break;
		case ComponentType::BOX_COLLIDER:		{ return "Box Collider"; }		break;
		case ComponentType::SPHERE_COLLIDER:	{ return "Sphere Collider"; }	break;
		case ComponentType::CAPSULE_COLLIDER:	{ return "Capsule Collider"; }	break;
		case ComponentType::PLAYER_CONTROLLER:	{ return "Player Controller"; } break;
		case ComponentType::BULLET_BEHAVIOR:	{ return "Bullet Behavior"; }	break;
		case ComponentType::PROP_BEHAVIOR:		{ return "Prop Behavior"; }		break;
		case ComponentType::CAMERA_BEHAVIOR:	{ return "Camera Behavior"; }	break;
		case ComponentType::GATE_BEHAVIOR:		{ return "Gate Behavior"; }		break;
		case ComponentType::SCRIPT: 			{ return "Script"; } 			break;
		}

		return "NONE";
	}

	template <typename T>
	bool GetComponents(std::vector<T*>& componentsWithType)
	{
		ComponentType type = T::GetType();
		for (uint i = 0; i < components.size(); ++i)
		{
			if (components[i]->GetType() == type)
			{
				componentsWithType.push_back((T*)components[i]);
			}
		}

		return  (componentsWithType.empty()) ? false : true;
	}


public:
	std::vector<Component*>	components;
	std::vector<GameObject*> childs;

	GameObject* parent;
	C_Transform* transform;													// Don't know what to do with this. Maybe like Unity? Or have it like the rest of comps?


	bool is_master_root;												//
	bool is_scene_root;												// Will be set to true if this GameObject is M_Scene's scene root object.
	bool is_bone;
	bool to_delete;													// Will determine whether or not the GO should be deleted. See M_Scene's DeleteGameObject().

	OBB	 obb;
	AABB aabb;

	float3*	obb_vertices;
	float3*	aabb_vertices;

	bool show_bounding_boxes;

	bool isPrefab = false; //Defines if the object is part of a prefav
	uint prefabID = 0;	//Id of the prefab the game object is part of

private:
	uint32 uid;
	std::string name;
	bool isActive;
	bool isStatic;

	uint32 parent_uid;													// Only for Serialization purposes. Maybe will be repurposed later.

};

#endif // !__GAME_OBJECT_H__