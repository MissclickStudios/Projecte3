#ifndef __E_INSPECTOR_H__
#define __E_INSPECTOR_H__

#include "EditorPanel.h"

class GameObject;
class Component;
class C_Transform;
class C_Mesh;
class C_Material;
class C_Light;
class C_Camera;
class C_Animator;
class C_Animation;
class C_RigidBody;

class Resource;
class ModelSettings;
class MeshSettings;
class MaterialSettings;
class TextureSettings;
class AnimationSettings;

class E_Inspector : public EditorPanel
{
public:
	E_Inspector();
	~E_Inspector();

	bool Draw		(ImGuiIO& io) override;
	bool CleanUp	() override;

private:
	void DrawGameObjectInfo				(GameObject* selectedGameObject);
	void DrawComponents					(GameObject* selectedGameObject);

	void DrawTransformComponent			(C_Transform* cTransform);
	void DrawMeshComponent				(C_Mesh* cMesh);
	void DrawMaterialComponent			(C_Material* cMaterial);
	void DrawLightComponent				(C_Light* cLight);
	void DrawCameraComponent			(C_Camera* cCamera);
	void DrawAnimatorComponent			(C_Animator* cAnimator);
	void DrawAnimationComponent			(C_Animation* cAnimation);
	void DrawRigidBodyComponent			(C_RigidBody* cRigidBody);

	void AddComponentCombo				(GameObject* selectedGameObject);					// 
	void DeleteComponentPopup			(GameObject* selectedGameObject);					// 

	// ------- DRAW COMPONENT METHODS -------
	void DisplayTextureData				(C_Material* cMaterial);							// Will display the texture's width, height, depth...
	void TextureDisplay					(C_Material* cMaterial);							// Will display the texture as an image through Dear ImGui.

private:
	void DrawImportSettings				(Resource* selectedResource);

	void DrawModelImportSettings		(ModelSettings modelSettings);
	void DrawMeshImportSettings			(MeshSettings meshSettings);
	void DrawMaterialImportSettings		(MaterialSettings materialSettings);
	void DrawTextureImportSettings		(TextureSettings textureSettings);
	void DrawAnimationImportSettings	(AnimationSettings animationSettings);

private:
	bool		showDeleteComponentPopup;
	int			componentType;

	int			mapToDisplay;

	Component*	componentToDelete;
};

#endif // !__E_INSPECTOR_H__