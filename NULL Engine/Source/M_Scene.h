#ifndef __M_SCENE_H__
#define __M_SCENE_H__

#include <map>
#include <utility>

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
class R_Scene;
class R_Model;
class R_Texture;
class R_Shader;
class GameObject;
class C_Camera;

struct ModelNode;

typedef unsigned __int32 uint32;
typedef unsigned int uint;

class MISSCLICK_API M_Scene : public Module
{
public:
	M_Scene(bool isActive = true);
	~M_Scene();

	bool			Init(ParsonNode& config) override;
	bool			Start() override;
	UpdateStatus	Update(float dt) override;
	UpdateStatus	PostUpdate(float dt) override;
	bool			CleanUp() override;

	bool			SaveConfiguration(ParsonNode& root) const override;
	bool			LoadConfiguration(ParsonNode& root) override;

public:																													// --- GAME OBJECTS METHODS ---
	bool			SaveScene(const char* sceneName = nullptr) const;													// If no name is given the scene_root node's name will be used.
	bool			LoadScene(const char* path);																		// For now asks for full path

	void			SaveCurrentScene();
	bool			SaveSceneAs(const char* sceneName = nullptr);														// To be called from editor
	bool			NewScene();																							// Opens a new scene
	
	void			UpdateSceneFromRoot(GameObject* root);
	void			RefreshSceneTransforms();

	void			LoadResourceIntoScene(Resource* resource);

	GameObject*		LoadPrefabIntoScene(ParsonNode* a, GameObject* parent);
	void			LoadPrefabObject(GameObject* gameObject, ParsonNode* node);
	
	GameObject*		InstantiatePrefab(uint prefabID,GameObject* parent,float3 position,Quat rotation,float3 globalPos = float3::zero);

	std::vector<GameObject*>*	GetGameObjects();
	
	GameObject*		CreateGameObject(const char* name = nullptr, GameObject* parent = nullptr);							// 
	void			DeleteGameObject(GameObject* gameObject, int index = -1);											// 

	void			AddGameObjectToVector(GameObject* gameObject);														//

	void			AddGameObjectToScene(GameObject* gameObject,GameObject* parent = nullptr);							// Will integrate the game object into scene as well as its childs
	void			AddGameObjectChildrenToScene(GameObject* gameObject);												// Recursive call to add the children

	GameObject*		GenerateGameObjectsFromModel(const R_Model* rModel, const float3& scale = float3::zero);			//
	bool			ApplyTextureToSelectedGameObject(const uint32& textureUid);											//

	void			CreateComponentsFromModelNode(const ModelNode& modelNode, GameObject* gameObject);
	void			CreateAnimationComponentFromModel(const R_Model* rModel, GameObject* gameObject);

public:																													// --- MASTER ROOT & SCENE ROOT METHODS ---
	void			CreateMasterRoot();																					// 
	void			DeleteMasterRoot();																					// 
	GameObject*		GetMasterRoot() const;																				// 

	const char*		GetCurrentScene() const;
	const char*		GetCurrentSceneJson() const;
	void			CreateSceneRoot(const char* sceneName);																//
	GameObject*		GetSceneRoot() const;																				//
	void			SetSceneRoot(GameObject* gameObject);																//
	void			ChangeSceneName(const char* newName);																//

	void			CreateSceneCamera(const char* cameraName);
	C_Camera*		GetCullingCamera() const;
	void			SetCullingCamera(C_Camera* cullingCamera);
	bool			GameObjectIsInsideCullingCamera(GameObject* gameObject);
	GameObject*		GetGameObjectByUID(uint32 UID);
	GameObject*		GetGameObjectByName(const char* name);
public:																													// --- SELECTED GAME OBJECT METHODS ---
	GameObject*		GetSelectedGameObject() const;																		// 
	void			SetSelectedGameObject(GameObject* gameObject);														// 
	void			DeleteSelectedGameObject();																			// 

public:																													// --- SELECT THROUGH RAYCAST
	void			SelectGameObjectThroughRaycast(const LineSegment& ray);
	void			GetRaycastHits(const LineSegment& ray, std::map<float, GameObject*>& hits);
	void			GetFaces(const std::vector<float>& vertices, std::vector<Triangle>& faces);

public:																													// --- SCENE LIGHT METHODS
	void			AddSceneLight(GameObject* light);																	// Set the light in the scene with the given
	bool			CheckSceneLight();																					// Check if there is a light already in the scene
	bool			SceneHasLights();
	void			GetAllLights(std::vector<GameObject*>& allLights);													// Return the light in the scene
	void			GetDirLights(std::vector<GameObject*>& dirLights);
	void			GetPointLights(std::vector<GameObject*>& pointLights);

	void			NextRoom();
	void			ScriptChangeScene(const std::string& sceneName);
	void			DoSceneTransition(R_Shader* screenShader, float transitionSpeed);

	void			HandleCopyGO();
	void			ResolveScriptGoPointer(const uint32 uid, GameObject** object);
	
	void			ShowFPS();

public:
	bool creditsMainMenu = false;

private:
	std::vector<GameObject*> gameObjects;
	std::multimap<uint32, std::pair<uint32, std::string>> models;														// Models currently loaded on scene and their correspondent GO.
																														// <goUID, <rUID, rAssetName>>

	GameObject*		masterRoot;																							// Root of everything. Parent of all scenes.
	GameObject*		sceneRoot;																							// Root of the current scene.
	GameObject*		animationRoot;																						// TMP Just for the 3rd Assignment Delivery
	GameObject*		selectedGameObject;																					// Represents the game object that's currently being selected.
	GameObject*		showFps;

	C_Camera*		cullingCamera;																						// Culling Camera

	std::string		currentScene;

	std::vector<std::pair<uint32, GameObject**>> toAdd;

private:
	bool			nextScene = false;
	bool			noTransitions = false;
	float			transitionProgresion;
	std::string		nextSceneName;

	friend class EngineApplication;
};

#endif // !__M_SCENE_H__