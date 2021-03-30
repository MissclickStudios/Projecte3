#ifndef __M_SCENE_H__
#define __M_SCENE_H__

#include <map>

#include "MathGeoLib/include/Geometry/LineSegment.h"
#include "MathGeoLib/include/Geometry/Triangle.h"
#include "Module.h"
#include "LevelGenerator.h"

namespace math
{
	class float3;
}

class ParsonNode;
class Primitive;
class Resource;
class R_Scene;
class R_Model;
class R_Texture;
class GameObject;
class C_Camera;
//class LevelGenerator;

struct ModelNode;

typedef unsigned __int32 uint32;

class NULL_API M_Scene : public Module
{
public:
	M_Scene(bool isActive = true);
	~M_Scene();

	bool			Init				(ParsonNode& config) override;
	bool			Start				() override;
	UpdateStatus	Update				(float dt) override;
	UpdateStatus	PostUpdate			(float dt) override;
	bool			CleanUp				() override;

	bool			SaveConfiguration	(ParsonNode& root) const override;
	bool			LoadConfiguration	(ParsonNode& root) override;

public:																														// --- GAME OBJECTS METHODS ---
	bool			SaveScene							(const char* sceneName = nullptr) const;							// If no name is given the scene_root node's name will be used.
	bool			LoadScene							(const char* path);													// For now asks for full path

	void			LoadResourceIntoScene				(Resource* resource);

	void LoadPrefabIntoScene(ParsonNode* a);
	void LoadPrefabObject(GameObject* gameObject,ParsonNode* node);

	std::vector<GameObject*>* GetGameObjects			();
	
	GameObject*		CreateGameObject					(const char* name = nullptr, GameObject* parent = nullptr);			// 
	void			DeleteGameObject					(GameObject* gameObject, uint index = -1);							// 
	
	GameObject*		GenerateGameObjectsFromModel		(const R_Model* rModel, const float3& scale = float3::zero);		//
	bool			ApplyTextureToSelectedGameObject	(const uint32& textureUid);											//

	void			CreateComponentsFromModelNode		(const ModelNode& modelNode, GameObject* gameObject);
	void			CreateAnimationComponentFromModel	(const R_Model* rModel, GameObject* gameObject);

public:																														// --- MASTER ROOT & SCENE ROOT METHODS ---
	void			CreateMasterRoot					();																	// 
	void			DeleteMasterRoot					();																	// 
	GameObject*		GetMasterRoot						() const;															// 

	const char*		GetCurrentScene()const;
	void			CreateSceneRoot						(const char* sceneName);											//
	GameObject*		GetSceneRoot						() const;															//
	void			SetSceneRoot						(GameObject* gameObject);											//
	void			ChangeSceneName						(const char* newName);												//

	void			CreateSceneCamera					(const char* cameraName);
	C_Camera*		GetCullingCamera					() const;
	void			SetCullingCamera					(C_Camera* cullingCamera);
	bool			GameObjectIsInsideCullingCamera		(GameObject* gameObject);
	GameObject*		GetGameObjectByUID					(uint32 uid);
	GameObject*		GetGameObjectByName(const char* name);
public:																														// --- SELECTED GAME OBJECT METHODS ---
	GameObject*		GetSelectedGameObject				() const;															// 
	void			SetSelectedGameObject				(GameObject* gameObject);											// 
	void			DeleteSelectedGameObject			();																	// 

public:																														// --- SELECT THROUGH RAYCAST
	void			SelectGameObjectThroughRaycast		(const LineSegment& ray);
	void			GetRaycastHits						(const LineSegment& ray, std::map<float, GameObject*>& hits);
	void			GetFaces							(const std::vector<float>& vertices, std::vector<Triangle>& faces);

public:	
	bool			CheckSceneLight();	//Check if there is a light already in the scene
	std::vector<GameObject*> GetAllLights();	//Return the light in the scene
	void			AddSceneLight(GameObject* light);	//Set the light in the scene with the given
	std::vector<GameObject*> GetDirLights();
	std::vector<GameObject*> GetPointLights();

	void NextRoom();

private:
	std::vector<GameObject*>		gameObjects;																			// 
	std::multimap<uint32, std::pair<uint32, std::string>> models;															// Models currently loaded on scene and their correspondent GO.

	GameObject*						masterRoot;																				// Root of everything. Parent of all scenes.
	GameObject*						sceneRoot;																				// Root of the current scene.
	GameObject*						animationRoot;																			// TMP Just for the 3rd Assignment Delivery
	GameObject*						selectedGameObject;																		// Represents the game object that's currently being selected.

	C_Camera*						cullingCamera;																			// Culling Camera

	std::vector<Primitive*>			primitives;

	LevelGenerator					level;

	std::string currentScene;

public:
	bool nextScene = false;

};

#endif // !__M_SCENE_H__