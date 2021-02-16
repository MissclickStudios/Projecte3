#ifndef __M_SCENE_H__
#define __M_SCENE_H__

#include <map>

#include "MathGeoLib/include/Geometry/LineSegment.h"
#include "MathGeoLib/include/Geometry/Triangle.h"
#include "Module.h"

namespace math
{
	class float3;
}

class ParsonNode;
class Primitive;
class Resource;
class R_Model;
class R_Texture;
class GameObject;
class C_Camera;

struct ModelNode;

typedef unsigned __int32 uint32;

class M_Scene : public Module
{
public:
	M_Scene(bool is_active = true);
	~M_Scene();

	bool			Init				(ParsonNode& config) override;
	bool			Start				() override;
	UPDATE_STATUS	Update				(float dt) override;
	UPDATE_STATUS	PostUpdate			(float dt) override;
	bool			CleanUp				() override;

	bool			SaveConfiguration	(ParsonNode& root) const override;
	bool			LoadConfiguration	(ParsonNode& root) override;

public:																														// --- GAME OBJECTS METHODS ---
	bool			SaveScene							(const char* scene_name = nullptr) const;							// If no name is given the scene_root node's name will be used.
	bool			LoadScene							(const char* path);													// 

	void			LoadResourceIntoScene				(Resource* resource);

	std::vector<GameObject*>* GetGameObjects			();
	
	GameObject*		CreateGameObject					(const char* name = nullptr, GameObject* parent = nullptr);			// 
	void			DeleteGameObject					(GameObject* game_object, uint index = -1);							// 
	
	void			GenerateGameObjectsFromModel		(const uint32& model_UID, const float3& scale = float3::zero);		//
	bool			ApplyTextureToSelectedGameObject	(const uint32& texture_UID);										//

	void			CreateComponentsFromModelNode		(const ModelNode& model_node, GameObject* game_object);
	void			CreateAnimationComponentFromModel	(const R_Model* r_model, GameObject* game_object);

public:																														// --- MASTER ROOT & SCENE ROOT METHODS ---
	void			CreateMasterRoot					();																	// 
	void			DeleteMasterRoot					();																	// 
	GameObject*		GetMasterRoot						() const;															// 

	void			CreateSceneRoot						(const char* scene_name);											//
	GameObject*		GetSceneRoot						() const;															//
	void			SetSceneRoot						(GameObject* game_object);											//
	void			ChangeSceneName						(const char* new_name);												//

	void			CreateSceneCamera					(const char* camera_name);
	C_Camera*		GetCullingCamera					() const;
	void			SetCullingCamera					(C_Camera* culling_camera);
	bool			GameObjectIsInsideCullingCamera		(GameObject* game_object);

public:																														// --- SELECTED GAME OBJECT METHODS ---
	GameObject*		GetSelectedGameObject				() const;															// 
	void			SetSelectedGameObject				(GameObject* game_object);											// 
	void			DeleteSelectedGameObject			();																	// 

public:																														// --- SELECT THROUGH RAYCAST
	void			SelectGameObjectThroughRaycast		(const LineSegment& ray);
	void			GetRaycastHits						(const LineSegment& ray, std::map<float, GameObject*>& hits);
	void			GetFaces							(const std::vector<float>& vertices, std::vector<Triangle>& faces);

private:
	void			HandleDebugInput();
	void			DebugSpawnPrimitive(Primitive* p);

private:
	std::vector<GameObject*>	game_objects;																				// 

	GameObject*					master_root;																				// Root of everything. Parent of all scenes.
	GameObject*					scene_root;																					// Root of the current scene.
	GameObject*					animation_root;																				// TMP Just for the 3rd Assignment Delivery
	GameObject*					selected_game_object;																		// Represents the game object that's currently being selected.

	C_Camera*					culling_camera;																				// Culling Camera

	std::vector<Primitive*>		primitives;
};

#endif // !__M_SCENE_H__
