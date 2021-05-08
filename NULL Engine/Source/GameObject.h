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
class C_UI;

struct MeshRenderer;
struct CuboidRenderer;
struct SkeletonRenderer;
struct Light;

typedef unsigned int		uint;
typedef unsigned __int32	uint32;

class MISSCLICK_API GameObject
{
public:
	GameObject();
	GameObject(std::string name , bool isActive = true, bool isStatic = false);
	~GameObject();

	bool			Start				();
	bool			Update				();
	bool			CleanUp				();

	bool			SaveState			(ParsonNode& root) const;
	bool			LoadState			(ParsonNode& root);

public:
	void			FreeComponents						();												// 
	void			FreeChilds							();												// 
	
	void			UpdateBoundingBoxes					();												// 
	AABB			GetAABB								() const;										// 
	float3*			GetAABBVertices						() const;										// 
	void			GetRenderers						(std::vector<MeshRenderer>& meshRenderers, std::vector<CuboidRenderer>& cuboidRenderers, 
															std::vector<SkeletonRenderer>& skeletonRenderers); // TODO: Get them elsewhere. Scene maybe?

public:																									// --- PARENT/CHILDS METHODS
	bool			SetParent							(GameObject* newParent);						// 

	bool			AddChild							(GameObject* child);							// Adds the given child to this GO's childs. Also dels. it from prev. parent's childs.
	bool			NewChildIsOwnParent					(GameObject* child);							// Rets. true if passed child is being added to one of it's children or ch. of chs.
	bool			DeleteChild							(GameObject* child);							// Deletes the given child from childs. Returns false upon not finding the child.
	bool			HasChilds							() const;

	void			GetAllChilds						(std::vector<GameObject*>& children);						// 
	void			GetAllChilds						(std::map<std::string, GameObject*>& children);				// 
	void			GetAllChilds						(std::unordered_map<std::string, GameObject*>& children);	// 
	GameObject*		FindChild							(const char* childName);									// 

	void			GetAllParents						(std::vector<GameObject*>& parents);					// Will return all the GO's parents until parent->isSceneRoot = true;

	void			SetAsPrefab							(uint _prefabID);								//Prefabs

public:																									// --- GAME OBJECT GETTERS AND SETTERS
	uint32			GetUID								() const;										//
	const char*		GetName								() const;										// 
	bool			IsActive							() const;										// 
	bool			IsStatic							() const;										// 

	void			ForceUID							(const uint32& UID);							// 
	void			SetName								(const char* newName);							// 
	void			SetIsActive							(const bool setTo);								// 
	void			SetIsStatic							(const bool setTo);								// 
	void			SetChildsIsActive					(const bool setTo, GameObject* parent);			// 
	void			SetChildsIsStatic					(const bool setTo, GameObject* parent);			// 

	bool			GetMaintainThroughScenes			() const;										// 
	void			SetMaintainThroughScenes			(const bool setTo);								// 

	uint32			GetParentUID						() const;										// 
	void			SetParentUID						(const uint32& parentUID);						// 

public:																									// --- COMPONENT GETTERS AND SETTERS
	Component*		CreateComponent						(ComponentType type, bool addComponent = true);	// Creates a component of the given type and adds it to the components vector.
	bool			DeleteComponent						(Component* componentToDelete);					// Deletes the given component from the Components vector. Returs False on ERROR.
	void			ReplaceComponent					(Component* newComponent);						// 

	const std::vector<Component*>&	GetAllComponents	() const;										// 
	bool							GetAllComponents	(std::vector<Component*>& components) const;	// 

	void*			GetScript							(const char* scriptName);						//
	void GetUiComponents(std::vector<C_UI*>& uiComponents);
	C_UI* GetUiComponent();
	void SetUiChildOrder(int index);

	template<typename T>																				// --- GET COMPONENT TEMPLATED METHOD
	T* GetComponent() const																				// 
	{																									//  
		ComponentType type = T::GetType();																// 
																										// 
		if (type == ComponentType::TRANSFORM)															// Optimization to avoid having to look for the transform component each time.
		{																								// 
			return (T*)transform;																		// This can be applied as there will be ONLY ONE TRANSFORM COMPONENT PER GO.
		}																								// 
																										// 
		for (uint i = 0; i < components.size(); ++i)													// 
		{																								// 
			if (components[i] != nullptr)																// 
			{																							// 
				if (components[i]->GetType() == type)													// 
				{																						// 
					return (T*)components[i];															// 
				}																						// 
			}																							// 
		}																								// 
																										// 
		return nullptr;																					// 
	}																									// ----------------------------------

	template <typename T>																				// --- GET COMPONENT NAME TEMPLATED METHOD
	const char* GetComponentName() const																// 
	{																									// 
		ComponentType type = T::GetType();																// 
		switch (type)																					// 
		{																								// 
		case ComponentType::TRANSFORM:			{ return "Transform"; }			break;					// 
		case ComponentType::MESH:				{ return "Mesh"; }				break;					// 
		case ComponentType::MATERIAL:			{ return "Material"; }			break;					// 
		case ComponentType::LIGHT:				{ return "Light"; }				break;					// 
		case ComponentType::CAMERA:				{ return "Camera"; }			break;					// 
		case ComponentType::ANIMATOR:			{ return "Animator"; }			break;					// 
		case ComponentType::ANIMATION:			{ return "Animation"; }			break;					// 
		case ComponentType::AUDIOSOURCE:		{ return "Audio Source"; }		break;					// 
		case ComponentType::AUDIOLISTENER:		{ return "Audio Listener"; }	break;					// 
		case ComponentType::RIGIDBODY:			{ return "RigidBody"; }			break;					// 
		case ComponentType::BOX_COLLIDER:		{ return "Box Collider"; }		break;					// 
		case ComponentType::SPHERE_COLLIDER:	{ return "Sphere Collider"; }	break;					// 
		case ComponentType::CAPSULE_COLLIDER:	{ return "Capsule Collider"; }	break;					// 
		case ComponentType::PLAYER_CONTROLLER:	{ return "Player Controller"; } break;					// 
		case ComponentType::BULLET_BEHAVIOR:	{ return "Bullet Behavior"; }	break;					// 
		case ComponentType::PROP_BEHAVIOR:		{ return "Prop Behavior"; }		break;					// 
		case ComponentType::CAMERA_BEHAVIOR:	{ return "Camera Behavior"; }	break;					// 
		case ComponentType::GATE_BEHAVIOR:		{ return "Gate Behavior"; }		break;					// 
		case ComponentType::SCRIPT: 			{ return "Script"; } 			break;					// 
		case ComponentType::ANIMATOR2D:			{ return "Animator2D"; } 		break;					// 
		case ComponentType::NAVMESH_AGENT:		{ return "NavMesh Agent"; }		break;					// 
		case ComponentType::PARTICLES:			{ return "Particles"; }			break;					//
		}																								// 
																										// 
		return "NONE";																					// 
	}																									// ---------------------------------------

	template <typename T>																				// --- GET COMPONENTS TEMPLATED METHOD
	bool GetComponents(std::vector<T*>& componentsWithType)												// 
	{																									// 
		ComponentType type = T::GetType();																// 
		for (uint i = 0; i < components.size(); ++i)													// 
		{																								//
			if (components[i]->GetType() == type)														// 
			{																							//
				componentsWithType.push_back((T*)components[i]);										// 
			}																							//
		}																								//
																										//
		return !componentsWithType.empty();																// 
	}																									// -----------------------------------

public:
	std::vector<Component*>		components;																// All the Components that this GameObject has.
	std::vector<GameObject*>	childs;																	// All the GameObjects that are parented to this GameObject.

	GameObject*		parent;																				// GameObject to which this GameObject is parented to.
	C_Transform*	transform;																			// Transform of this GO. Maybe like Unity? Or have it like the rest of components?


	bool			isMasterRoot;																		// Will be set to true if this GameObject is M_Scenes's master root object.
	bool			isSceneRoot;																		// Will be set to true if this GameObject is M_Scene's scene root object.
	bool			isBone;																				// Will be set to true if this GameObject is an 3D Animation bone.
	bool			toDelete;																			// Will determine if this GO should be deleted. See M_Scene's DeleteGameObject().

	OBB				obb;																				// Object Bounding Box that represents the cuboidal space that this GO occupies.
	AABB			aabb;																				// Axis Aligned Bounding Box. Same as the above but is not affected by rotations.

	float3*			obb_vertices;																		// Array containing each of the vertices that compose the OBB.
	float3*			aabb_vertices;																		// Array containing each of the vertices that compose the AABB.

	bool			show_bounding_boxes;																// Will determine whether or not this GO's OBB and AABB are shown in the viewport.

	bool			isPrefab = false;																	// Defines if the object is part of a prefav
	uint			prefabID = 0;																		// Id of the prefab the game object is part of

	//Navigation
	uint			navigationArea = 0;																	// Describes the navigation of the current game object
	bool			isNavigable = false;																// Defines is the object is navigable.

private:
	uint32			uid;																				// Unique IDentifier of this GameObject.
	std::string		name;																				// Name of this GameObject.
	bool			isActive;																			// Will determine whether or not this GameObject will be updated and rendered.
	bool			isStatic;																			// Will determine whether or not this GameObject will be subject of transformations.

	uint32			parent_uid;																			// Only for Serialization purposes. Maybe will be repurposed later.

	bool			maintainThroughScenes;																// Will determine whether or not the GO will be deleted when changing scenes.



};

#endif // !__GAME_OBJECT_H__