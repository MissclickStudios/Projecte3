#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include "MathGeoBoundingBox.h"
#include "Component.h"

class ParsonNode;
class C_Transform;

struct MeshRenderer;
struct CuboidRenderer;
struct SkeletonRenderer;

typedef unsigned __int32 uint32;

class GameObject
{
public:
	GameObject();
	GameObject(std::string name , bool is_active = true, bool is_static = false);
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
	void			GetRenderers						(std::vector<MeshRenderer>& mesh_renderers, std::vector<CuboidRenderer>& cuboid_renderers, 
															std::vector<SkeletonRenderer>& skeleton_renderers); // TODO: Get them elsewhere. Scene maybe?

public:																									// --- PARENT/CHILDS METHODS
	bool			SetParent							(GameObject* new_parent);

	bool			AddChild							(GameObject* child);							// Adds the given child to this GO's childs. Also dels. it from prev. parent's childs.
	bool			NewChildIsOwnParent					(GameObject* child);							// Rets. true if passed child is being added to one of it's children or ch. of chs.
	bool			DeleteChild							(GameObject* child);							// Deletes the given child from childs. Returns false upon not finding the child.
	bool			HasChilds							() const;

	void			GetAllChilds						(std::vector<GameObject*>& childs);	
	void			GetAllChilds						(std::map<std::string, GameObject*>& childs);
	void			GetAllChilds						(std::unordered_map<std::string, GameObject*>& childs);
	GameObject*		FindChild							(const char* child_name);

public:																									// --- GAME OBJECT GETTERS AND SETTERS
	uint32			GetUID								() const;										//
	const char*		GetName								() const;										// 
	bool			IsActive							() const;										// 
	bool			IsStatic							() const;										// 

	void			ForceUID							(const uint32& UID);							// 
	void			SetName								(const char* new_name);							// 
	void			SetIsActive							(const bool& set_to);							// 
	void			SetIsStatic							(const bool& set_to);							// 
	void			SetChildsIsActive					(const bool& set_to, GameObject* parent);		// 
	void			SetChildsIsStatic					(const bool& set_to, GameObject* parent);		// 

	uint32			GetParentUID						() const;
	void			SetParentUID						(const uint32& parent_UID);

public:																									// --- COMPONENT GETTERS AND SETTERS
	Component*		CreateComponent						(COMPONENT_TYPE type);							// Creates a component of the given type and adds it to the components vector.
	bool			DeleteComponent						(Component* component_to_delete);				// Deletes the given component from the Components vector. Returs False on ERROR.
	
	const std::vector<Component*>&	GetAllComponents	() const;										// 
	bool							GetAllComponents	(std::vector<Component*>& components) const;	// 

	template<typename T>
	T* GetComponent() const
	{
		COMPONENT_TYPE type = T::GetType();

		if (type == COMPONENT_TYPE::TRANSFORM)															// Optimization to avoid having to look for the transform component each time.
		{
			return (T*)transform;																		// This can be applied as there will be ONLY ONE TRANSFORM COMPONENT PER GO.
		}

		for (uint i = 0; i < components.size(); ++i)
		{
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
		COMPONENT_TYPE type = T::GetType();
		switch (type)
		{
		case COMPONENT_TYPE::TRANSFORM: { return "Transform"; } break;
		case COMPONENT_TYPE::MESH:		{ return "Mesh"; }		break;
		case COMPONENT_TYPE::MATERIAL:	{ return "Material"; }	break;
		case COMPONENT_TYPE::LIGHT:		{ return "Light"; }		break;
		case COMPONENT_TYPE::CAMERA:	{ return "Camera"; }	break;
		case COMPONENT_TYPE::ANIMATOR:	{ return "Animator"; }	break;
		case COMPONENT_TYPE::ANIMATION:	{ return "Animation"; } break;
		}

		return "NONE";
	}

	template <typename T>
	bool GetComponents(std::vector<T*>& components_with_type)
	{
		COMPONENT_TYPE type = T::GetType();
		for (uint i = 0; i < components.size(); ++i)
		{
			if (components[i]->GetType() == type)
			{
				components_with_type.push_back((T*)components[i]);
			}
		}

		return  (components_with_type.empty()) ? false : true;
	}

public:
	std::vector<Component*>		components;
	std::vector<GameObject*>	childs;

	GameObject*					parent;
	C_Transform*				transform;													// Don't know what to do with this. Maybe like Unity? Or have it like the rest of comps?

	bool						is_master_root;												//
	bool						is_scene_root;												// Will be set to true if this GameObject is M_Scene's scene root object.
	bool						is_bone;
	bool						to_delete;													// Will determine whether or not the GO should be deleted. See M_Scene's DeleteGameObject().

	OBB							obb;
	AABB						aabb;

	float3*						obb_vertices;
	float3*						aabb_vertices;

	bool						show_bounding_boxes;

private:
	uint32						uid;
	std::string					name;
	bool						is_active;
	bool						is_static;

	uint32						parent_uid;													// Only for Serialization purposes. Maybe will be repurposed later.
};

#endif // !__GAME_OBJECT_H__