#ifndef __E_INSPECTOR_H__
#define __E_INSPECTOR_H__

#include "EditorPanel.h"
#include "TextEditor.h"
#include <vector>

class GameObject;
class Component;
class C_Transform;
class C_Mesh;
class C_Material;
class C_Light;
class C_Camera;
class C_Animator;
class C_Animation;
class C_AudioSource;
class C_AudioListener;
class C_RigidBody;
class C_BoxCollider;
class C_SphereCollider;
class C_CapsuleCollider;
class C_Canvas;
class C_Script;
class C_PlayerController;
class C_BulletBehavior;
class C_PropBehavior;
class C_CameraBehavior;
class C_GateBehavior;
class C_Canvas;
class C_ParticleSystem;
class C_UI_Image;
class C_UI_Text;

class Resource;
class R_Shader;
class R_Texture;
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
	void DrawAudioSourceComponent		(C_AudioSource* cAudioSource);
	void DrawAudioListenerComponent		(C_AudioListener* cAudioListener);
	void DrawRigidBodyComponent			(C_RigidBody* cRigidBody);
	void DrawBoxColliderComponent		(C_BoxCollider* cCollider);
	void DrawSphereColliderComponent	(C_SphereCollider* cCollider);
	void DrawCapsuleColliderComponent	(C_CapsuleCollider* cCollider);
	void DrawParticleSystemComponent	(C_ParticleSystem* cCanvas);
	void DrawCanvasComponent			(C_Canvas* cCanvas);
	void DrawUIImageComponent			(C_UI_Image* image);
	void DrawUITextComponent			(C_UI_Text* text);
	void DrawScriptComponent			(C_Script* cScript);
	void DrawPlayerControllerComponent	(C_PlayerController* controller);
	void DrawBulletBehaviorComponent	(C_BulletBehavior* behavior);
	void DrawPropBehaviorComponent		(C_PropBehavior* behavior);
	void DrawCameraBehaviorComponent	(C_CameraBehavior* behavior); 
	void DrawGateBehaviorComponent		(C_GateBehavior* behavior);

	void AddComponentCombo				(GameObject* selectedGameObject);					// 
	void DeleteComponentPopup			(GameObject* selectedGameObject);					// 
	void AddUIComponent					(GameObject* selectedGameObject, ComponentType type);

	void TextEditorWindow();
	void CallTextEditor(C_Material* cMaterial);

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
	bool		showTextEditorWindow;
	bool		showSaveEditorPopup;
	int			componentType;

	int			mapToDisplay;

	Component*	componentToDelete;

	//Shader inspector utilities

	std::vector<R_Shader*>	allShaders;
	R_Shader*				shaderToRecompile;
	TextEditor				editor;
	std::string				fileToEdit;
	std::string				shaderName;

	std::vector<R_Texture*>	allTextures;
	std::string				texName;

};

#endif // !__E_INSPECTOR_H__