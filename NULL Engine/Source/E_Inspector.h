#ifndef __E_INSPECTOR_H__
#define __E_INSPECTOR_H__

#include <vector>

#include "EditorPanel.h"
#include "TextEditor.h"

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
class C_UI_Button;
class C_2DAnimator;
class C_NavMeshAgent;

class ResourceBase;
class Resource;
class R_Shader;
class R_Texture;

class AnimatorTrack;
class AnimatorClip;

class Emitter;

class ModelSettings;
class MeshSettings;
class MaterialSettings;
class TextureSettings;
class AnimationSettings;

struct ParticleModule;

class E_Inspector : public EditorPanel
{
public:
	E_Inspector();
	~E_Inspector();

	bool Draw		(ImGuiIO& io) override;
	bool CleanUp	() override;

private:																										// --- DRAW COMPONENT METHODS ---
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
	void DrawUIButtonComponent			(C_UI_Button* button);
	//void DrawPlayerControllerComponent	(C_PlayerController* controller);
	//void DrawBulletBehaviorComponent	(C_BulletBehavior* behavior);
	//void DrawPropBehaviorComponent		(C_PropBehavior* behavior);
	//void DrawCameraBehaviorComponent	(C_CameraBehavior* behavior); 
	//void DrawGateBehaviorComponent		(C_GateBehavior* behavior);
	void DrawAnimator2DComponent		(C_2DAnimator* cAnimator);
	void DrawNavMeshAgentComponent		(C_NavMeshAgent* cNavMeshAgent);

private:																														// --- DRAW COMPONENT UTILITY METHODS ---
	void AddComponentCombo				(GameObject* selectedGameObject);														// 
	void DeleteComponentPopup			(GameObject* selectedGameObject);														// 
	void AddUIComponent					(GameObject* selectedGameObject, ComponentType type);

	// COMPONENT BASICS		--------
	void DrawBasicSettings				(Component* component, const char* state = nullptr);

	// MESH COMPONENT		--------
	void DisplayMeshSelector			(C_Mesh* cMesh, std::map<std::string, ResourceBase>& meshBases);
	
	// MATERIAL COMPONENT	--------
	void DisplayMaterial				(C_Material* cMaterial);
	void DisplayMaterialSelector		(C_Material* cMaterial, std::map<std::string, ResourceBase>& materialBases);

	void DisplayShader					(C_Material* cMaterial);
	void DisplayShaderSelector			(C_Material* cMaterial, std::map<std::string, ResourceBase>& shaderBases);
	
	void DisplayTexture					(C_Material* cMaterial);
	void DisplayTextureSelector			(C_Material* cMaterial, std::map<std::string, ResourceBase>& textureBases);
	void DisplayTextureData				(C_Material* cMaterial);																// Will display the texture's width, height, depth...
	void TextureImageDisplay			(C_Material* cMaterial);																// Will display the texture as an image through Dear ImGui.

	// ANIMATOR COMPONENT	--------
	void DisplayAnimatorSettings		(C_Animator* cAnimator);
	void DisplayAnimatorControls		(C_Animator* cAnimator);
	void DisplayTrackViewer				(C_Animator* cAnimator);
	void DisplayTrackStats				(AnimatorTrack* track, AnimatorClip* clip);

	void DisplayClipManager				(C_Animator* cAnimator);
	void ClipCreatorWindow				(C_Animator* cAnimator);
	void ClipEditorWindow				(C_Animator* cAnimator);
	
	void DisplayTrackManager			(C_Animator* cAnimator);
	void TrackCreatorWindow				(C_Animator* cAnimator);
	void TrackEditorWindow				(C_Animator* cAnimator);

	// SHADER COMPONENT		--------
	void TextEditorWindow				();
	void CallTextEditor					(C_Material* cMaterial);

	// RIGID BODY COMPONENT --------
	void RigidBodyFilterCombo			(C_RigidBody* cRigidBody);

	// PARTICLE SYSTEM COMPONENT ---
	void DisplayParticleSystemControls	(C_ParticleSystem* cParticleSystem);
	void DisplayEmitterInstances		(C_ParticleSystem* cParticleSystem);

	void DisplayParticleModules			(Emitter* emitter);
	void DisplayEmitterBase				(ParticleModule* pModule);
	void DisplayEmitterSpawn			(ParticleModule* pModule);
	void DisplayEmitterArea				(ParticleModule* pModule);
	void DisplayParticleMovement		(ParticleModule* pModule);
	void DisplayParticleColor			(ParticleModule* pModule);
	void DisplayParticleLifetime		(ParticleModule* pModule);
	void DisplayParticleRotation		(ParticleModule* pModule);
	void DisplayParticleSize			(ParticleModule* pModule);
	void DisplayParticleBillboarding	(ParticleModule* pModule);

	void ScriptSelectCombo(C_Script*& cScript, const char* previewValue, bool swapForCurrent);

private:
	//void DrawImportSettings				(Resource* selectedResource);

	//void DrawModelImportSettings		(ModelSettings modelSettings);
	//void DrawMeshImportSettings			(MeshSettings meshSettings);
	//void DrawMaterialImportSettings		(MaterialSettings materialSettings);
	//void DrawTextureImportSettings		(TextureSettings textureSettings);
	//void DrawAnimationImportSettings	(AnimationSettings animationSettings);

private:
	GameObject* shownGameObject = nullptr;
	bool		lockGameObject	= false;
	
	bool		showDeleteComponentPopup;
	Component*	componentToDelete;
	
	bool		showTextEditorWindow;
	bool		showSaveEditorPopup;
	int			componentType;

	int			billboardingType;
	int			moduleType;

	// Animator Insector Variables
	bool		trackWasDeleted;
	bool		clipWasDeleted;

	//Shader inspector utilities
	R_Shader*	shaderToRecompile;
	TextEditor	editor;
	std::string	fileToEdit;
};

#endif // !__E_INSPECTOR_H__