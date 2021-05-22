#include <vector>
#include <string>

#include "MathGeoTransform.h"
#include "Profiler.h"
#include "Color.h"
#include "AnimatorClip.h"
#include "Spritesheet.h"

#include "MC_Time.h"

#include "EngineApplication.h"
#include "M_Renderer3D.h"
#include "M_Editor.h"
#include "M_Audio.h"
#include "M_FileSystem.h"
#include "M_ResourceManager.h"
#include "M_UISystem.h"
#include "M_Scene.h"
#include "M_EngineScriptManager.h"
#include "M_Physics.h"

#include "GameObject.h"
#include "Component.h"
#include "C_Transform.h"
#include "C_Mesh.h"
#include "C_Material.h"
#include "C_Light.h"
#include "C_Camera.h"
#include "C_Animator.h"
#include "C_Animation.h"
#include "C_AudioSource.h"
#include "C_AudioListener.h"
#include "C_RigidBody.h"
#include "C_BoxCollider.h"
#include "C_SphereCollider.h"
#include "C_CapsuleCollider.h"
//#include "C_PlayerController.h"
//#include "C_BulletBehavior.h"
#include "C_ParticleSystem.h"
//#include "C_PropBehavior.h"
//#include "C_CameraBehavior.h"
//#include "C_GateBehavior.h"
#include "C_Canvas.h"
#include "C_UI.h"
#include "C_UI_Image.h"
#include "C_UI_Text.h"
#include "C_UI_Button.h"
#include "C_UI_Checkbox.h"
#include "C_Script.h"
#include "C_2DAnimator.h"
#include "C_NavMeshAgent.h"

#include "ResourceBase.h"
#include "R_Mesh.h"
#include "R_Texture.h"
#include "R_Animation.h"
#include "R_Shader.h"
#include "R_Script.h"
#include "R_ParticleSystem.h"

#include "I_Shaders.h"

#include "Emitter.h"

#include "E_Inspector.h"
#include "MathGeoLib/include/Math/float3.h"

#include <fstream>
#include "CoreDllHelpers.h"
#include "MemoryManager.h"

#define MAX_VALUE 100000
#define MIN_VALUE -100000

E_Inspector::E_Inspector() : EditorPanel("Inspector"),
showDeleteComponentPopup	(false),
componentToDelete			(nullptr),
showTextEditorWindow		(false),
showSaveEditorPopup			(false),
componentType				(0),
shaderToRecompile			(nullptr),
trackWasDeleted				(false),
clipWasDeleted				(false)
{

}

E_Inspector::~E_Inspector()
{
	componentToDelete = nullptr;
}

bool E_Inspector::Draw(ImGuiIO& io)
{
	bool ret = true;
	OPTICK_CATEGORY("E_Inspector Draw", Optick::Category::Editor)

	ImGui::Begin("Inspector");

	SetIsHovered();

	// --- IS LOCKED ---
	ImGui::Checkbox("Is Locked", &lockGameObject);
	
	if (!lockGameObject)
	{
		shownGameObject = EngineApp->editor->GetSelectedGameObjectThroughEditor();

		/*if (selected != nullptr)
			shownGameObject = selected;
		else
			shownGameObject = nullptr;*/
	}
	else
	{
		if(shownGameObject != nullptr)
			if (shownGameObject->toDelete)
			{
				lockGameObject = false;

				GameObject* selected = EngineApp->editor->GetSelectedGameObjectThroughEditor();

				if (selected != nullptr)
					shownGameObject = selected;
			}
	}
	

	if (shownGameObject != nullptr && !shownGameObject->isMasterRoot && !shownGameObject->isSceneRoot)
	{	
		DrawGameObjectInfo(shownGameObject);
		DrawComponents(shownGameObject);
		TextEditorWindow();
		ImGui::Separator();

		AddComponentCombo(shownGameObject);

		if (showDeleteComponentPopup)
		{
			DeleteComponentPopup(shownGameObject);
		}
	}

	//ImGui::Text("WantCaptureMouse: %d", io.WantCaptureMouse);
	//ImGui::Text("WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
	//ImGui::Text("WantTextInput: %d", io.WantTextInput);
	//ImGui::Text("WantSetMousePos: %d", io.WantSetMousePos);
	//ImGui::Text("NavActive: %d, NavVisible: %d", io.NavActive, io.NavVisible);

	ImGui::End();

	return ret;
}

bool E_Inspector::CleanUp()
{
	bool ret = true;

	return ret;
}

// --- INSPECTOR METHODS ---
void E_Inspector::DrawGameObjectInfo(GameObject* selectedGameObject)
{
	// --- IS ACTIVE ---
	bool gameObjectIsActive = selectedGameObject->IsActive();
	if (ImGui::Checkbox("Is Active", &gameObjectIsActive))
	{
		selectedGameObject->SetIsActive(gameObjectIsActive);
	}

	ImGui::SameLine();

	// --- GAME OBJECT'S NAME ---
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.33f);
	static char buffer[64];
	strcpy_s(buffer, selectedGameObject->GetName());
	if (ImGui::InputText("Name", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		selectedGameObject->SetName(buffer);
	}

	ImGui::SameLine(); HelpMarker("Press ENTER to Rename");

	ImGui::SameLine();

	// --- IS STATIC ---
	bool isStatic = selectedGameObject->IsStatic();
	//bool isStatic = true;
	if (ImGui::Checkbox("Is Static", &isStatic))
	{
		selectedGameObject->SetIsStatic(isStatic);
	}
	
	// --- TAG ---
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.33f);
	static char tagCombo[64] = { "Untagged\0Work\0In\0Progress" };
	static int currentTag = 0;
	ImGui::Combo("Tag", &currentTag, tagCombo);

	ImGui::SameLine(218.0f);

	// --- LAYER ---
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.33f);
	static char layerCombo[64] = { "Default\0Work\0In\0Progress" };
	static int currentLayer = 0;
	ImGui::Combo("Layer", &currentLayer, layerCombo);

	ImGui::Separator();
	
	if (!selectedGameObject->isPrefab)	// --- PREFAB ---
	{
		if (ImGui::Button("Create Prefab"))
		{
		
			App->resourceManager->CreatePrefab(selectedGameObject);
			//App->resourceManager->RefreshProjectDirectories(); //Should only refresh prefabs
		}
	}
	else
	{
		if (ImGui::Button("Update Prefab"))
			App->resourceManager->UpdatePrefab(selectedGameObject);
	}

	ImGui::SameLine(ImGui::GetWindowWidth() * 0.51f);

	bool maintain = selectedGameObject->GetMaintainThroughScenes();
	if (ImGui::Checkbox("Maintain Through Scenes", &maintain)) { selectedGameObject->SetMaintainThroughScenes(maintain); }

	ImGui::Separator();
}

void E_Inspector::DrawComponents(GameObject* selectedGameObject)
{
	if (selectedGameObject == nullptr)
	{
		LOG("[ERROR] Editor Inspector: Could not draw the selected GameObject's components! Error: Selected GameObject was nullptr.");
		return;
	}
	
	for (uint i = 0; i < selectedGameObject->components.size(); ++i)
	{
		Component* component = selectedGameObject->components[i];
		
		if (component == nullptr)
		{
			continue;
		}
		
		ComponentType type = component->GetType();	
		switch (type)
		{
		case ComponentType::TRANSFORM:			{ DrawTransformComponent((C_Transform*)component); }				break;
		case ComponentType::MESH:				{ DrawMeshComponent((C_Mesh*)component); }							break;
		case ComponentType::MATERIAL:			{ DrawMaterialComponent((C_Material*)component); }					break;
		case ComponentType::LIGHT:				{ DrawLightComponent((C_Light*)component); }						break;
		case ComponentType::CAMERA:				{ DrawCameraComponent((C_Camera*)component); }						break;
		case ComponentType::ANIMATOR:			{ DrawAnimatorComponent((C_Animator*)component); }					break;
		case ComponentType::ANIMATION:			{ DrawAnimationComponent((C_Animation*)component); }				break;
		case ComponentType::AUDIOSOURCE:		{ DrawAudioSourceComponent((C_AudioSource*)component); }			break;
		case ComponentType::AUDIOLISTENER:		{ DrawAudioListenerComponent((C_AudioListener*)component); }		break;
		case ComponentType::RIGIDBODY:			{ DrawRigidBodyComponent((C_RigidBody*)component); }				break;
		case ComponentType::BOX_COLLIDER:		{ DrawBoxColliderComponent((C_BoxCollider*)component); }			break;
		case ComponentType::SPHERE_COLLIDER:	{ DrawSphereColliderComponent((C_SphereCollider*)component); }		break;
		case ComponentType::CAPSULE_COLLIDER:	{ DrawCapsuleColliderComponent((C_CapsuleCollider*)component); }	break;
		case ComponentType::PARTICLE_SYSTEM:	{ DrawParticleSystemComponent((C_ParticleSystem*)component); }		break;
		case ComponentType::CANVAS:				{ DrawCanvasComponent((C_Canvas*)component); }						break;
		case ComponentType::UI_IMAGE:			{ DrawUIImageComponent((C_UI_Image*)component); }					break;
		case ComponentType::UI_TEXT:			{ DrawUITextComponent((C_UI_Text*)component); }						break;
		case ComponentType::SCRIPT:				{ DrawScriptComponent((C_Script*)component); }						break;
		case ComponentType::UI_BUTTON:			{ DrawUIButtonComponent((C_UI_Button*)component); }					break;
		//case ComponentType::PLAYER_CONTROLLER:	{ DrawPlayerControllerComponent((C_PlayerController*)component); }	break;
		//case ComponentType::BULLET_BEHAVIOR:	{ DrawBulletBehaviorComponent((C_BulletBehavior*)component); }		break;
		//case ComponentType::PROP_BEHAVIOR:		{ DrawPropBehaviorComponent((C_PropBehavior*)component); }			break;
		//case ComponentType::CAMERA_BEHAVIOR:	{ DrawCameraBehaviorComponent((C_CameraBehavior*)component); }		break;
		//case ComponentType::GATE_BEHAVIOR:		{ DrawGateBehaviorComponent((C_GateBehavior*)component); }			break;
		case ComponentType::ANIMATOR2D:			{ DrawAnimator2DComponent((C_2DAnimator*)component); }				break;
		case ComponentType::NAVMESH_AGENT:		{ DrawNavMeshAgentComponent((C_NavMeshAgent*)component); }			break;
		case ComponentType::UI_CHECKBOX:		{ DrawUICheckboxComponent((C_UI_Checkbox*)component); }				break;
		}
		if (type == ComponentType::NONE)
		{
			LOG("[WARNING] Selected GameObject %s has a non-valid component!", selectedGameObject->GetName());
		}
	}
}

void E_Inspector::DrawTransformComponent(C_Transform* cTransform)
{
	bool show = true;																				// Dummy bool to delete the component related with the collpsing header.
	if (ImGui::CollapsingHeader("Transform", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cTransform != nullptr)
		{
			// --- IS ACTIVE ---
			bool transformIsActive = cTransform->IsActive();
			if (ImGui::Checkbox("Transform Is Active", &transformIsActive))
			{
				//transform->SetIsActive(transform_is_active);
				cTransform->SetIsActive(transformIsActive);
			}

			ImGui::Separator();

			// --- POSITION ---
			ImGui::Text("Position");

			ImGui::SameLine(100.0f);

			float3 position = cTransform->GetLocalPosition();
			if (ImGui::DragFloat3("P", (float*)&position, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
			{
				cTransform->SetLocalPosition(position);
			}

			// --- ROTATION ---
			ImGui::Text("Rotation");

			ImGui::SameLine(100.0f);

			/*float3 rotation = transform->GetLocalEulerRotation();
			if (ImGui::DragFloat3("R", (float*)&rotation, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
			{
				transform->SetLocalEulerRotation(rotation);
			}*/

			float3 rotation = cTransform->GetLocalEulerRotation() * RADTODEG;
			if (ImGui::DragFloat3("R", (float*)&rotation, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
			{	
				cTransform->SetLocalRotation(rotation * DEGTORAD);
			}

			// --- SCALE ---
			ImGui::Text("Scale");

			ImGui::SameLine(100.0f);

			float3 scale = cTransform->GetLocalScale();
			if (ImGui::DragFloat3("S", (float*)&scale, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
			{
				cTransform->SetLocalScale(scale);
			}
		}

		if (!show)
		{
			LOG("[ERROR] Transform components cannot be deleted!");
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawMeshComponent(C_Mesh* cMesh)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Mesh", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (!show)
		{
			componentToDelete			= cMesh;
			showDeleteComponentPopup	= true;

			return;
		}

		if (cMesh == nullptr)
		{
			LOG("[ERROR] Could not get the Mesh Component from %s Game Object!", cMesh->GetOwner()->GetName());
			ImGui::Separator();

			return;
		}

		bool meshIsActive			= cMesh->IsActive();
			
		static uint numVertices		= 0;
		static uint numNormals		= 0;
		static uint numTexCoords	= 0;
		static uint numIndices		= 0;
		static uint numBones		= 0;

		bool showWireframe			= cMesh->GetShowWireframe();
		bool showBoundingBox		= cMesh->GetShowBoundingBox();
		bool drawVertNormals		= cMesh->GetDrawVertexNormals();
		bool drawFaceNormals		= cMesh->GetDrawFaceNormals();
		
		bool outlineMesh			= cMesh->GetOutlineMesh();
		float outlineThickness		= cMesh->GetOutlineThickness();
		Color outlineColor			= cMesh->GetOutlineColor();
		
		static std::map<std::string, ResourceBase> meshBases;

		// --- IS ACTIVE ---
		if (ImGui::Checkbox("Mesh Is Active", &meshIsActive))
		{
			cMesh->SetIsActive(meshIsActive);
		}

		ImGui::Separator();

		// --- MESH SELECTOR ---
		DisplayMeshSelector(cMesh, meshBases);

		ImGui::Separator();

		// --- MESH DATA ---
		ImGui::TextColored(Green.C_Array(), "Mesh Data:");

		cMesh->GetMeshData(numVertices, numNormals, numTexCoords, numIndices, numBones);

		ImGui::Text("Vertices:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "  %u",	numVertices);
		ImGui::Text("Normals:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "   %u",	numNormals);
		ImGui::Text("Tex Coords:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "%u",		numTexCoords);
		ImGui::Text("Indices:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "   %u",	numIndices);
		ImGui::Text("Bones: ");			ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "    %u",	numBones);

		ImGui::Separator();

		// --- DRAW MODE ---
		ImGui::TextColored(Green.C_Array(), "Draw Mode:");

		if (ImGui::Checkbox("Show Wireframe", &showWireframe))							{ cMesh->SetShowWireframe(showWireframe); }
		if (ImGui::Checkbox("Show Bounding Box", &showBoundingBox))						{ cMesh->SetShowBoundingBox(showBoundingBox); }
		if (ImGui::Checkbox("Draw Vertex Normals", &drawVertNormals))					{ cMesh->SetDrawVertexNormals(drawVertNormals); }
		if (ImGui::Checkbox("Draw Face Normals", &drawFaceNormals))						{ cMesh->SetDrawFaceNormals(drawFaceNormals); }

		ImGui::Separator();

		// --- OUTLINE MODE ---
		ImGui::TextColored(Green.C_Array(), "Outline Mesh:");

		if (ImGui::Checkbox("Outline Mesh", &outlineMesh))								{ cMesh->SetOutlineMesh(outlineMesh); }
		if (ImGui::SliderFloat("Outline Thickness", &outlineThickness, 0.0f , 4.0f))	{ cMesh->SetOutlineThickness(outlineThickness); }
		if (ImGui::ColorEdit4("Outline Color", outlineColor.C_Array()))					{ cMesh->SetOutlineColor(outlineColor); }

		ImGui::Separator();
	}
}

void E_Inspector::DrawMaterialComponent(C_Material* cMaterial)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Material", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (!show)
		{
			componentToDelete			= cMaterial;
			showDeleteComponentPopup	= true;

			return;
		}
		
		if (cMaterial == nullptr)
		{
			LOG("[ERROR] Could not get the Material Component from %s Game Object!", cMaterial->GetOwner()->GetName());
			ImGui::Separator();
			return;
		}

		// --- MATERIAL GENERAL SETTINGS ---
		bool materialIsActive = cMaterial->IsActive();
		if (ImGui::Checkbox("Material Is Active", &materialIsActive))	{ cMaterial->SetIsActive(materialIsActive); }

		ImGui::Separator();

		// --- MATERIAL ---
		ImGui::Separator();
		ImGui::Separator();
		
		DisplayMaterial(cMaterial);

		ImGui::Separator();

		// --- SHADER ---
		ImGui::Separator();
		ImGui::Separator();

		DisplayShader(cMaterial);

		ImGui::Separator();

		// --- TEXTURE ---
		ImGui::Separator();
		
		DisplayTexture(cMaterial);

		ImGui::Separator();
	}
}

void E_Inspector::DrawLightComponent(C_Light* cLight)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Light", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cLight != nullptr)
		{
			bool lightIsActive = cLight->IsActive();
			if (ImGui::Checkbox("Light Is Active", &lightIsActive))
			{
				cLight->SetIsActive(lightIsActive);
			}
			
			ImGui::Separator();
			float4 diffuse;
			float4 ambient;
			float4 specular;
			float3 direction;
			switch (cLight->GetLightType())
			{
			case LightType::DIRECTIONAL: 

				diffuse = (float4)&cLight->GetDirectionalLight()->diffuse;
				ambient = (float4)&cLight->GetDirectionalLight()->ambient;
				specular = (float4)&cLight->GetDirectionalLight()->specular;
				direction = cLight->GetDirectionalLight()->GetDirection();
				if (ImGui::DragFloat4("Diffuse", (float*)&diffuse, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
				{
					cLight->GetDirectionalLight()->diffuse = (Color&)diffuse;
				}
				if (ImGui::DragFloat4("Ambient", (float*)&ambient, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
				{
					cLight->GetDirectionalLight()->ambient = (Color&)ambient;
				}
				if (ImGui::DragFloat4("Specular", (float*)&specular, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
				{
					cLight->GetDirectionalLight()->specular = (Color&)specular;
				}
				break;
			case LightType::POINTLIGHT: 
				diffuse = (float4)&cLight->GetPointLight()->diffuse;
				ambient = (float4)&cLight->GetPointLight()->ambient;
				specular = (float4)&cLight->GetPointLight()->specular;
				if (ImGui::DragFloat4("Diffuse", (float*)&diffuse, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
				{
					cLight->GetPointLight()->diffuse = (Color&)diffuse;
				}
				if (ImGui::DragFloat4("Ambient", (float*)&ambient, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
				{
					cLight->GetPointLight()->ambient = (Color&)ambient;
				}
				if (ImGui::DragFloat4("Specular", (float*)&specular, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
				{
					cLight->GetPointLight()->specular = (Color&)specular;
				}
				break;
			case LightType::SPOTLIGHT: break;
			case LightType::NONE: break;
			}


			
		}

		if (!show)
		{
			componentToDelete				= cLight;
			showDeleteComponentPopup		= true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawCameraComponent(C_Camera* cCamera)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Camera", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cCamera != nullptr)
		{
			bool cameraIsActive = cCamera->IsActive();
			if (ImGui::Checkbox("Camera Is Active", &cameraIsActive))
			{
				cCamera->SetIsActive(cameraIsActive);
			}

			ImGui::Separator();

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Camera Flags:");
			
			bool cameraIsCulling = cCamera->IsCulling();
			if (ImGui::Checkbox("Culling", &cameraIsCulling))
			{
				cCamera->SetIsCulling(cameraIsCulling);
			}

			bool cameraIsOrthogonal = cCamera->OrthogonalView();
			if (ImGui::Checkbox("Orthogonal", &cameraIsOrthogonal))
			{
				cCamera->SetOrthogonalView(cameraIsOrthogonal);
			} 

			bool frustumIsHidden = cCamera->FrustumIsHidden();
			if (ImGui::Checkbox("Hide Frustum", &frustumIsHidden))
			{
				cCamera->SetFrustumIsHidden(frustumIsHidden);
			}

			ImGui::Separator();

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Frustum Settings:");

			float nearPlaneDistance = cCamera->GetNearPlaneDistance();
			if (ImGui::SliderFloat("Near Plane", &nearPlaneDistance, 0.1f, 1000.0f, "%.3f", 0))
			{
				cCamera->SetNearPlaneDistance(nearPlaneDistance);
			}

			float farPlaneDistance = cCamera->GetFarPlaneDistance();
			if (ImGui::SliderFloat("Far Plane", &farPlaneDistance, 0.1f, 1000.0f, "%.3f", 0))
			{
				cCamera->SetFarPlaneDistance(farPlaneDistance);
			}
			
			int fov			= (int)cCamera->GetVerticalFOV();
			uint minFov	= 0;
			uint maxFov	= 0;
			cCamera->GetMinMaxFOV(minFov, maxFov);
			if (ImGui::SliderInt("FOV", &fov, minFov, maxFov, "%d"))
			{
				cCamera->SetVerticalFOV((float)fov);
			}
			
			ImGui::Separator();

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Camera Selection:");

			if (ImGui::Button("Set as Current Camera"))
			{
				EngineApp->editor->SetCurrentCameraThroughEditor(cCamera);
			}

			if (ImGui::Button("Return to Master Camera"))
			{
				EngineApp->editor->SetMasterCameraThroughEditor();
			}
		}

		if (!show)
		{
			componentToDelete				= cCamera;
			showDeleteComponentPopup		= true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawAnimatorComponent(C_Animator* cAnimator)								// TODO: Segment this Method in Multiple Smaller Methods.
{
	OPTICK_CATEGORY("Animator Inspector", Optick::Category::Animation);
	bool show = true;
	if (ImGui::CollapsingHeader("Animator", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cAnimator != nullptr)
		{
			std::string animator = cAnimator->GetAnimatorStateAsString();
			DrawBasicSettings((Component*)cAnimator, animator.c_str());
			CoreCrossDllHelpers::CoreReleaseString(animator);

			// --- DISPLAY
			if (ImGui::BeginTabBar("AnimatorTabBar", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("Settings & Controls"))
				{
					DisplayAnimatorSettings(cAnimator);

					ImGui::EndTabItem();
				}
				
				if (ImGui::BeginTabItem("Track Manager"))
				{
					DisplayTrackManager(cAnimator);

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Clip Manager"))
				{
					DisplayClipManager(cAnimator);

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}
		
		if (!show)
		{
			componentToDelete				= cAnimator;
			showDeleteComponentPopup		= true;
		}
		
		ImGui::Separator();
	}
}

void E_Inspector::DrawAnimationComponent(C_Animation* cAnimation)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Animation", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cAnimation != nullptr)
		{
			bool isActive = cAnimation->IsActive();
			if (ImGui::Checkbox("Is Active", &isActive)) { cAnimation->SetIsActive(isActive); }

			ImGui::Separator();

			ImGui::TextColored(Cyan.C_Array(), "Animation Settings:");

			ImGui::Separator();
		}

		if (!show)
		{
			componentToDelete				= cAnimation;
			showDeleteComponentPopup		= true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawAudioSourceComponent(C_AudioSource* cAudioSource)
{
	bool show = true;
	if (ImGui::CollapsingHeader(("Audio Source " + cAudioSource->GetEventName()).c_str(), &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cAudioSource != nullptr)
		{
			unsigned int currentEvent = cAudioSource->GetEventId();
			const std::string& currentEventName = cAudioSource->GetEventName();
			//cAudioSource->GetEvent(&currentEventName,&currentEvent);

			ImGui::Text("Event playing %s", currentEventName.c_str());
			ImGui::Text("Event id %u", currentEvent);

			if (ImGui::BeginCombo(("##Audio " + cAudioSource->GetEventName()).c_str(), currentEventName.c_str()))
			{
				for (auto it = App->audio->eventMap.cbegin(); it != App->audio->eventMap.cend(); ++it)
				{
					bool isSelected = (currentEventName == it->first);
					if (ImGui::Selectable(it->first.c_str(), isSelected)) {
							//currentEventName = it->first;
							currentEvent = it->second;
							cAudioSource->SetEvent(it->first, currentEvent);
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			static float volume = cAudioSource->GetVolume();
			if (ImGui::DragFloat(("Volume " + cAudioSource->GetEventName()).c_str(), &volume, 0.01f, 0.01f, 1.0f))
			{
				cAudioSource->SetVolume(volume);
			}

			if ((ImGui::Button(("Play " + cAudioSource->GetEventName()).c_str())))
			{
				cAudioSource->PlayFx(currentEvent);
				cAudioSource->isPlaying = false;
			}
			if ((ImGui::Button(("Stop " + cAudioSource->GetEventName()).c_str())))
			{
				cAudioSource->isPlaying = true;
				cAudioSource->StopFx(currentEvent);
			}
		}
		if (!show)
		{
			componentToDelete = cAudioSource;
			showDeleteComponentPopup = true;
		}
    }
    ImGui::Separator();
}

void E_Inspector::DrawRigidBodyComponent(C_RigidBody* cRigidBody)
{
	bool show = true;

	if (cRigidBody->IsStatic())
	{
		if (ImGui::CollapsingHeader("Static RigidBody", &show, ImGuiTreeNodeFlags_Leaf))
		{
			bool isActive = cRigidBody->IsActive();
			if (ImGui::Checkbox("RigidBody Is Active", &isActive))
				cRigidBody->SetIsActive(isActive);

			ImGui::SameLine();

			if (ImGui::Button("Make Dynamic"))
				cRigidBody->MakeDynamic();

			ImGui::Separator();

			RigidBodyFilterCombo(cRigidBody);

			ImGui::Separator();
			
			if (!show)
			{
				componentToDelete = cRigidBody;
				showDeleteComponentPopup = true;
			}

			ImGui::Separator();
		}
		return;
	}

	if (ImGui::CollapsingHeader("RigidBody", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cRigidBody != nullptr)
		{
			bool isActive = cRigidBody->IsActive();
			if (ImGui::Checkbox("RigidBody Is Active", &isActive))
				cRigidBody->SetIsActive(isActive);

			ImGui::SameLine();

			if (ImGui::Button("Make Static"))
				cRigidBody->MakeStatic();

			ImGui::Separator();

			RigidBodyFilterCombo(cRigidBody);

			ImGui::Separator();

			float mass = cRigidBody->GetMass();
			if (ImGui::InputFloat("Mass", &mass, 0, 0, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
				cRigidBody->SetMass(mass);
			float density = cRigidBody->GetDensity();
			if (ImGui::InputFloat("Denstity", &density, 0, 0, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
				cRigidBody->SetDensity(density);

			bool useGravity = cRigidBody->UsingGravity();
			if (ImGui::Checkbox("Use Gravity", &useGravity))
				cRigidBody->UseGravity(useGravity);
			bool isKinematic = cRigidBody->IsKinematic();
			if (ImGui::Checkbox("Is Kinematic", &isKinematic))
				cRigidBody->SetKinematic(isKinematic);

			if (ImGui::Button("Stop Inertia"))
				cRigidBody->StopInertia();

			if (ImGui::TreeNodeEx("Constrains"))
			{
				ImGui::Text("Freeze Position");
				ImGui::SameLine();

				bool x, y, z;
				cRigidBody->FrozenPositions(x, y, z);

				if (ImGui::Checkbox("X", &x))
					cRigidBody->FreezePositionX(x);
				ImGui::SameLine();
				if (ImGui::Checkbox("Y", &y))
					cRigidBody->FreezePositionY(y);
				ImGui::SameLine();
				if (ImGui::Checkbox("Z", &z))
					cRigidBody->FreezePositionZ(z);

				ImGui::Text("Freeze Rotation");
				ImGui::SameLine();

				cRigidBody->FrozenRotations(x, y, z);

				if (ImGui::Checkbox("X##", &x))
					cRigidBody->FreezeRotationX(x);
				ImGui::SameLine();
				if (ImGui::Checkbox("Y##", &y))
					cRigidBody->FreezeRotationY(y);
				ImGui::SameLine();
				if (ImGui::Checkbox("Z##", &z))
					cRigidBody->FreezeRotationZ(z);

				ImGui::TreePop();
			}

			if (ImGui::TreeNodeEx("Info"))
			{
				float speed = cRigidBody->GetSpeed();
				ImGui::InputFloat("Speed", &speed, 0, 0, 4, ImGuiInputTextFlags_ReadOnly);

				float3 vel = cRigidBody->GetLinearVelocity();
				float velocity[3] = { vel.x, vel.y,vel.z };
				ImGui::InputFloat3("Velocity", velocity, 4, ImGuiInputTextFlags_ReadOnly);

				vel = cRigidBody->GetAngularVelocity();
				float angVelocity[3] = { vel.x, vel.y,vel.z };
				ImGui::InputFloat3("Angulat Velocity", angVelocity, 4, ImGuiInputTextFlags_ReadOnly);

				bool isSleeping = cRigidBody->IsSleeping();
				ImGui::Checkbox("Is Sleeping", &isSleeping);

				ImGui::TreePop();
			}
		}

		if (!show)
		{
			componentToDelete = cRigidBody;
			showDeleteComponentPopup = true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawBoxColliderComponent(C_BoxCollider* cCollider)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Box Collider", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cCollider != nullptr)
		{
			bool isActive = cCollider->IsActive();
			if (ImGui::Checkbox("Collider is Active##1", &isActive))
				cCollider->SetIsActive(isActive);

			ImGui::SameLine();

			bool showCollider = cCollider->ToShowCollider();
			if (ImGui::Checkbox("Show Collider##1", &showCollider))
				cCollider->SetShowCollider(showCollider);

			ImGui::Separator();

			std::string* a = cCollider->GetFil();
			if(a)
				ImGui::Text(a->c_str());

			bool isTrigger = cCollider->IsTrigger();
			if (ImGui::Checkbox("Is Trigger##1", &isTrigger))
				cCollider->SetTrigger(isTrigger);

			float3 size = cCollider->Size();
			float s[3] = { size.x, size.y, size.z };
			if (ImGui::InputFloat3("Size##1", s, 4, ImGuiInputTextFlags_EnterReturnsTrue))
				cCollider->SetSize(s[0], s[1], s[2]);

			float3 center = cCollider->GetCenter();
			float c[3] = { center.x, center.y, center.z };
			if (ImGui::InputFloat3("Center##1", c, 4, ImGuiInputTextFlags_EnterReturnsTrue))
				cCollider->SetCenter(c[0], c[1], c[2]);

		}

		if (!show)
		{
			componentToDelete = cCollider;

			showDeleteComponentPopup = true;
		}

	}
}

void E_Inspector::DrawCanvasComponent(C_Canvas* cCanvas)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Canvas", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cCanvas != nullptr)
		{
			//bool isActive = cCanvas->IsActive();
			//if (ImGui::Checkbox("Canvas Is Active", &isActive)) { cCanvas->SetIsActive(isActive); }

			//bool isInvisible = cCanvas->IsInvisible();
			//if (ImGui::Checkbox("Canvas Is Invisible", &isInvisible)) { cCanvas->SetIsInvisible(isInvisible); }
			if (ImGui::Checkbox("Debug draw", &cCanvas->debugDraw))
			{
				if (cCanvas->debugDraw)
					cCanvas->ResetUi();
				else
					cCanvas->cachedObjects.clear();
			}
				/*cCanvas->debugDraw = */
			ImGui::Separator();

			ImGui::TextColored(Cyan.C_Array(), "Canvas Settings:");

			ImGui::Separator();

			// --- RECT ---
			float2 size = { cCanvas->GetRect().w, cCanvas->GetRect().h };

			if (ImGui::DragFloat2("Rect", (float*)&size, 0.005f, 0.0f, 0.0f, "%.3f", NULL))
			{
				if (size.x < 0)
					size.x = 0;
				if (size.y < 0)
					size.y = 0;
					
				
				cCanvas->SetSize(size);
			}

			ImGui::Separator();

			if (ImGui::TreeNode("Reorder Ui children"))
			{
				// Simple reordering
				HelpMarker(
					"Reordering Ui children "
					"The rendering order is from first to last");
				std::vector<GameObject*>& children = cCanvas->GetOwner()->childs;
				for (int n = 0; n < children.size(); ++n)
				{
					GameObject* child = children[n];
					ImGui::Selectable(child->GetName());

					if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
					{
						int n_next = n + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
						if (n_next >= 0 && n_next < children.size())
						{
							children[n] = children[n_next];
							children[n_next] = child;
							ImGui::ResetMouseDragDelta();
						}
					}
				}
				ImGui::TreePop();
			}
		}

		if (!show)
		{
			componentToDelete = cCanvas;

			showDeleteComponentPopup = true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawSphereColliderComponent(C_SphereCollider* cCollider)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Sphere Collider", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cCollider != nullptr)
		{
			bool isActive = cCollider->IsActive();
			if (ImGui::Checkbox("Collider is Active##2", &isActive)) { cCollider->SetIsActive(isActive); }

			ImGui::Separator();

			bool isTrigger = cCollider->IsTrigger();
			if (ImGui::Checkbox("Is Trigger##2", &isTrigger))
				cCollider->SetTrigger(isTrigger);

			float radius = cCollider->Radius();
			if (ImGui::InputFloat("Radius##2", &radius, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
				cCollider->SetRadius(radius);

			float3 center = cCollider->GetCenter();
			float c[3] = { center.x, center.y, center.z };
			if (ImGui::InputFloat3("Center##2", c, 4, ImGuiInputTextFlags_EnterReturnsTrue))
				cCollider->SetCenter(c[0], c[1], c[2]);
		}

		if (!show)
		{
			componentToDelete = cCollider;
			showDeleteComponentPopup = true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawAudioListenerComponent(C_AudioListener* cAudioListener)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Audio Listener", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cAudioListener != nullptr)
		{
	
		}
		if (!show)
		{
			componentToDelete = cAudioListener;
			showDeleteComponentPopup = true;
		}
		ImGui::Separator();
	}
}
void E_Inspector::DrawCapsuleColliderComponent(C_CapsuleCollider* cCollider)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Capsule Collider", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cCollider != nullptr)
		{
			bool isActive = cCollider->IsActive();
			if (ImGui::Checkbox("Collider is Active##3", &isActive)) { cCollider->SetIsActive(isActive); }

			ImGui::Separator();

			bool isTrigger = cCollider->IsTrigger();
			if (ImGui::Checkbox("Is Trigger##3", &isTrigger))
				cCollider->SetTrigger(isTrigger);

			float radius = cCollider->Radius();
			if (ImGui::InputFloat("Radius##3", &radius, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
				cCollider->SetRadius(radius);

			float height = cCollider->Height();
			if (ImGui::InputFloat("Height##3", &height, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
				cCollider->SetHeight(height);

			float3 center = cCollider->GetCenter();
			float c[3] = { center.x, center.y, center.z };
			if (ImGui::InputFloat3("Center##3", c, 4, ImGuiInputTextFlags_EnterReturnsTrue))
				cCollider->SetCenter(c[0], c[1], c[2]);
		}

		if (!show)
		{
			componentToDelete = cCollider;
			showDeleteComponentPopup = true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawParticleSystemComponent(C_ParticleSystem* cParticleSystem)
{
	bool a = true;
	if (a /*cParticleSystem->resource != nullptr*/)
	{
		bool show = true;
		if (ImGui::CollapsingHeader("Particle System", &show, ImGuiTreeNodeFlags_DefaultOpen))
		{
			DisplayParticleSystemControls(cParticleSystem);
			DisplayEmitterInstances(cParticleSystem);
		}

		if (!show)
		{
			componentToDelete = cParticleSystem;
			showDeleteComponentPopup = true;
		}
	}
	else
	{
		//assign a resource to the component
	}
}

void E_Inspector::DrawUIImageComponent(C_UI_Image* image)
{
	static bool show = true;
	if (ImGui::CollapsingHeader("Image", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool isActive = image->IsActive();
		if (ImGui::Checkbox("Image Is Active", &isActive)) { image->SetIsActive(isActive); }

		ImGui::Separator();

		// --- RECT ---
		float2 pos = { image->GetRect().x, image->GetRect().y };
		float2 size = { image->GetRect().w, image->GetRect().h };
		float offset = 0.1;

		C_Canvas* canvas = image->GetOwner()->parent->GetComponent<C_Canvas>();

		if (ImGui::DragFloat2("Image Size", (float*)&size, 0.005f, 0.0f, 0.0f, "%.3f", NULL))
		{
			if (size.x < 0)
				size.x = 0;
			if (size.y < 0)
				size.y = 0;

			image->SetW(size.x);
			image->SetH(size.y);
		}

		if (ImGui::DragFloat2("Image Pos", (float*)&pos, 0.005f, 0.0f, 0.0f, "%.3f", NULL))
		{
			image->SetX(pos.x);
			image->SetY(pos.y);
		}

		ImGui::ColorEdit3("Diffuse Color", (float*)&image->color, ImGuiColorEditFlags_NoAlpha);
		ImGui::SliderFloat("Diffuse Alpha", (float*)&image->color.a, 0.0f, 1.0f, "%.3f");

		if (ImGui::DragInt4("pixel TexCoord (x,y,w,h)", image->pixelCoord))
			image->textCoord = image->GetTexturePosition(image->pixelCoord[0], image->pixelCoord[1], image->pixelCoord[2], image->pixelCoord[3]);
	}

	ImGui::Separator();

}

void E_Inspector::DrawUITextComponent(C_UI_Text* text)
{
	static bool show = true;
	if (ImGui::CollapsingHeader("Text", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool isActive = text->IsActive();
		if (ImGui::Checkbox("Text Is Active", &isActive)) { text->SetIsActive(isActive); }

		ImGui::Separator();

		static char buffer[64];
		strcpy_s(buffer, text->GetText());
		if (ImGui::InputText("TextInput", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			text->SetText(buffer);
		}

		ImGui::SameLine(); HelpMarker("Press ENTER to Rename");

		ImGui::Separator();

		// --- RECT ---
		float2 pos = { text->GetRect().x, text->GetRect().y };
		float2 size = { text->GetRect().w, text->GetRect().h};
		Color newColor = text->GetColor();
		float offset = 0.1;

		C_Canvas* canvas = text->GetOwner()->parent->GetComponent<C_Canvas>();

		if (ImGui::DragFloat2("Text Size", (float*)&size, 0.005f, 0.0f, 0.0f, "%.3f", NULL))
		{
			if (size.x < 0)
				size.x = 0;
			if (size.y < 0)
				size.y = 0;

			text->SetW(size.x);
			text->SetH(size.y);
		}

		if (ImGui::DragFloat2("Text Pos", (float*)&pos, 0.005f, 0.0f, 0.0f, "%.3f", NULL))
		{
			text->SetX(pos.x);
			text->SetY(pos.y);
		}

		if (ImGui::DragFloat4("Color", (float*)&newColor, 0.01f, 0.0f, 1.0f, "%.3f", NULL))
		{
			text->SetColor(newColor);
		}

		float fontSize = text->GetFontSize() * 1000;
		if (ImGui::DragFloat("Font Size", (float*)&fontSize, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
		{
			text->SetFontSize(fontSize / 1000);
		}

		int hSB = text->GetHSpaceBetween();
		if (ImGui::DragInt("Horizontal Space Between", (int*)&hSB, 1, 0.0f, 0.0f, "%.3f", NULL))
		{
			text->SetHSpaceBetween(hSB);
		}

		float vSB = text->GetVSpaceBetween();
		if (ImGui::DragFloat("Vertical Space Between", (float*)&vSB, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
		{
			text->SetVSpaceBetween(vSB);
		}

		if (ImGui::Button("Font"))
		{
			text->GenerateTextureID("Assets/Fonts/main_menu.ttf");
		}
	}

	ImGui::Separator();

}
void E_Inspector::ScriptSelectCombo(C_Script*& cScript, const char* previewValue, bool swapForCurrent) 
{
	const std::map<std::string, std::string> scripts = ((M_EngineScriptManager*)EngineApp->scriptManager)->GetAviableScripts();

	std::string label = "##"; label += previewValue + cScript->GetDataName();
	if (ImGui::BeginCombo(label.c_str(), previewValue, ImGuiComboFlags_PopupAlignLeft))
	{
		//TODO: Forced and slow (improve)
		std::vector<C_Script*>goCurrentScripts;
		GameObject* owner = cScript->GetOwner();
		owner->GetComponents<C_Script>(goCurrentScripts);
		bool skip = false;
		for (std::map<std::string, std::string>::const_iterator it = scripts.cbegin(); it != scripts.cend(); ++it)
		{
			for (std::vector< C_Script*>::const_iterator cit = goCurrentScripts.cbegin(); cit != goCurrentScripts.cend(); ++cit)
			{
				if ((*it).first == (*cit)->GetDataName())
				{
					skip = true;
					break;
				}

			}
			if (skip) 
			{
				skip = false;
				continue;
			}

			if (ImGui::Selectable((*it).first.c_str(), false))
			{
				if (swapForCurrent) 
				{
					owner->DeleteComponent(cScript);
					cScript = (C_Script*)owner->CreateComponent(ComponentType::SCRIPT);
				}

				cScript->resource = (R_Script*)App->resourceManager->GetResourceFromLibrary((*it).second.c_str());
				for (int i = 0; i < cScript->resource->dataStructures.size(); ++i) {
					if ((*it).first == cScript->resource->dataStructures[i].first)
					{
						cScript->LoadData((*it).first.c_str(), cScript->resource->dataStructures[i].second);
						break;
					}
				}
			}

		}
		ImGui::EndCombo();
	}
}

void E_Inspector::DrawScriptComponent(C_Script* cScript)
{

	bool show = true;
	//REDO THIS -> s'ha de fer amb el resource !!!
	//Inspector if we don't have any script
	if (!cScript->HasData() && !(cScript->resource != nullptr))
	{
		if (ImGui::CollapsingHeader("Script", &show, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ScriptSelectCombo(cScript, "Select Script",false);
		}
		if (!show)
			cScript->GetOwner()->DeleteComponent(cScript);

		ImGui::Separator();
		return;
	}

	if (ImGui::CollapsingHeader((cScript->GetDataName() + " (Script)").c_str(), &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		//TODO: Show inspector variables
		ImGui::Text("Current Script: %s", cScript->GetDataName().c_str()); ImGui::SameLine(); 
		ScriptSelectCombo(cScript, "Change Script", true);
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		std::vector<InspectorScriptData> inspectorVariables = cScript->GetInspectorVariables();
		unsigned int numVariables = inspectorVariables.size();
		for (std::vector<InspectorScriptData>::iterator variable = inspectorVariables.begin(); variable != inspectorVariables.end(); ++variable,--numVariables)
		{
			switch ((*variable).variableType) 
			{
			case InspectorScriptData::DataType::INT:
				switch ((*variable).showAs) 
				{
				case InspectorScriptData::ShowMode::INPUT_INT:
					ImGui::InputInt((*variable).variableName.data(), (int*)(*variable).ptr); break;
				case InspectorScriptData::ShowMode::DRAGABLE_INT:
					ImGui::DragInt((*variable).variableName.data(), (int*)(*variable).ptr); break;
				case InspectorScriptData::ShowMode::SLIDER_INT:
					ImGui::SliderInt((*variable).variableName.data(), (int*)(*variable).ptr, (*variable).minSlider, (*variable).maxSlider); break;
				}
				break;
			case InspectorScriptData::DataType::BOOL:
				ImGui::Checkbox((*variable).variableName.data(), (bool*)(*variable).ptr);
				break;
			case InspectorScriptData::DataType::FLOAT:
				switch ((*variable).showAs)
				{
				case InspectorScriptData::ShowMode::INPUT_FLOAT:
					ImGui::InputFloat((*variable).variableName.data(), (float*)(*variable).ptr); break;
				case InspectorScriptData::ShowMode::DRAGABLE_FLOAT:
					ImGui::DragFloat((*variable).variableName.data(), (float*)(*variable).ptr); break;
				case InspectorScriptData::ShowMode::SLIDER_FLOAT:
					ImGui::SliderFloat((*variable).variableName.data(), (float*)(*variable).ptr, (*variable).minSlider, (*variable).maxSlider); break;
				}
				break;
			case InspectorScriptData::DataType::FLOAT3:
				switch ((*variable).showAs)
				{
				case InspectorScriptData::ShowMode::INPUT_FLOAT:
					ImGui::InputFloat3((*variable).variableName.data(), ((float3*)(*variable).ptr)->ptr()); break;
				case InspectorScriptData::ShowMode::DRAGABLE_FLOAT:
					ImGui::DragFloat3((*variable).variableName.data(), ((float3*)(*variable).ptr)->ptr()); break;
				case InspectorScriptData::ShowMode::SLIDER_FLOAT:
					ImGui::SliderFloat3((*variable).variableName.data(), ((float3*)(*variable).ptr)->ptr(), (*variable).minSlider, (*variable).maxSlider); break;
				}
				break;
			case InspectorScriptData::DataType::STRING: 
			{
				switch ((*variable).showAs)
				{
				case InspectorScriptData::ShowMode::NONE:
				{
					char buffer[128];
					strcpy_s(buffer, ((std::string*)(*variable).ptr)->c_str());
					if (ImGui::InputText((*variable).variableName.data(), buffer, IM_ARRAYSIZE(buffer)))
						*(std::string*)(*variable).ptr = buffer;
					break;
				}
				case InspectorScriptData::ShowMode::TEXT:
					ImGui::Text(((std::string*)(*variable).ptr)->c_str()); break;
				}
				break;
			}
			case InspectorScriptData::DataType::PREFAB:
				ImGui::Button((((Prefab*)(*variable).ptr)->name.empty()) ? "Prefab: NULL" : std::string("Prefab: " + ((Prefab*)(*variable).ptr)->name).data(), { ImGui::GetWindowWidth() * 0.55F , 0});
				if (ImGui::BeginDragDropTarget()) 
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAGGED_ASSET"))
					{
						if (App->fileSystem->GetFileExtension(*(const char**)payload->Data) == "prefab") 
						{
							std::string uidString;
							App->fileSystem->SplitFilePath(*(const char**)payload->Data, nullptr, &uidString, nullptr);
							unsigned int prefabUid = std::atoi(uidString.c_str());
							*(Prefab*)(*variable).ptr = EngineApp->resourceManager->prefabs[prefabUid];
						}

					}
					ImGui::EndDragDropTarget();
				}
				ImGui::SameLine();
				if (ImGui::Button(("Remove " + (*variable).variableName).c_str())) //Maybe variables with the same name of different scripts on the same gameobject collide for imgui...
				{
					(*(Prefab*)(*variable).ptr).name.clear();
					(*(Prefab*)(*variable).ptr).uid = 0;
					(*(Prefab*)(*variable).ptr).updateTime = 0;
				}
				break;
			case InspectorScriptData::DataType::GAMEOBJECT:
				ImGui::Button(((*variable).obj != nullptr && *(*variable).obj != nullptr) ? std::string("GameObject: "  + std::string((*(*variable).obj)->GetName())).data() : "GameObject: NULL", { ImGui::GetWindowWidth() * 0.55F , 0 });
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAGGED_NODE"))
					{
						GameObject* ptr = *(GameObject**)payload->Data;
						if (ptr != nullptr) 
						{
							*(*variable).obj = App->scene->GetGameObjectByUID(ptr->GetUID());
							//*(*variable).obj = ptr;
						}
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::SameLine();
				if (ImGui::Button(("Remove " + (*variable).variableName).c_str()))
					if ((*variable).obj != nullptr)
						*(*variable).obj = nullptr;
				break;
			case InspectorScriptData::DataType::VECTORSTRING:
			{
				if (ImGui::TreeNodeEx((*variable).variableName.c_str()))
				{
					//int draggedIndex = -1;
					std::vector<std::string>& stringVector = (*(std::vector<std::string>*)(*variable).ptr);
					for (int i = 0; i < stringVector.size(); ++i)
					{
						char buffer[128];
						strcpy_s(buffer, stringVector[i].c_str());
						std::string index = std::to_string(i);
						if (ImGui::InputText(((*variable).variableName + " " + index).c_str(), buffer, IM_ARRAYSIZE(buffer)))
							stringVector[i] = buffer;
						//-----------------------------------DRAG DROP----------------------------------------------------
						if (ImGui::BeginDragDropSource())
						{
							//draggedIndex = i;
							ImGui::SetDragDropPayload("STRING_VECTOR_NODE", /*&draggedIndex*/&i, sizeof(int), ImGuiCond_Once);
							ImGui::Text("Dragging %s", stringVector[i].c_str());

							ImGui::EndDragDropSource();
						}

						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("STRING_VECTOR_NODE"))
							{
								std::string tmp = stringVector[i];
								stringVector[i] = stringVector[*(int*)payload->Data];
								stringVector[*(int*)payload->Data] = tmp;
								ImGui::FocusWindow(NULL);
							}

							ImGui::EndDragDropTarget();
						}
						//-----------------------------------DRAG DROP----------------------------------------------------
						ImGui::SameLine();
						if (ImGui::Button(("Remove " + index).c_str()))
						{
							EngineApp->scriptManager->StringVecErase((*variable).ptr, i);
							--i;
						}
					}
					ImGui::Text("Add new string to vector");
					char buffer[128];
					strcpy_s(buffer, "");
					if (ImGui::InputText("New Element", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
						EngineApp->scriptManager->StringVecPushBackString((*variable).ptr, buffer);

					ImGui::TreePop();
				}
				break;
			}
			case InspectorScriptData::DataType::ENUM:
			{
				const std::map<std::string, std::map<int,std::string>> enumsMap = ((M_EngineScriptManager*)EngineApp->scriptManager)->GetInspectorEnums();
				const std::map<int, std::string>& enumMap = enumsMap.at((*variable).enumName);

				std::string selected = enumMap.at(*(int*)(*variable).ptr);
				if (ImGui::BeginCombo(std::string("##" + (*variable).variableName).c_str(), selected.c_str(), ImGuiComboFlags_PopupAlignLeft))
				{
					for (auto it = enumMap.cbegin(); it != enumMap.cend(); ++it)
					{
						if (ImGui::Selectable((*it).second.c_str(),(*it).second == selected))
							*(int*)(*variable).ptr = (*it).first;
					}
					ImGui::EndCombo();
				}
				ImGui::SameLine(); ImGui::Text((*variable).variableName.c_str());
				break;
			}
			}
			if (numVariables > 1) 
			{
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}
		}
	}
	if (!show)
	{
		componentToDelete = cScript;
		showDeleteComponentPopup = true;
	}
	ImGui::Separator();

}

void E_Inspector::DrawUIButtonComponent(C_UI_Button* button)
{
	static bool show = true;
	if (ImGui::CollapsingHeader("Button", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool isActive = button->IsActive();
		if (ImGui::Checkbox("Button Is Active", &isActive)) { button->SetIsActive(isActive); }

		ImGui::Separator();

		// --- RECT ---
		float2 pos = { button->GetRect().x, button->GetRect().y };
		float2 size = { button->GetRect().w, button->GetRect().h };

		//C_Canvas* canvas = button->GetOwner()->parent->GetComponent<C_Canvas>();

		if (ImGui::DragFloat2("Button Size", (float*)&size, 0.005f, 0.0f, 0.0f, "%.3f", NULL))
		{
			if (size.x < 0)
				size.x = 0;
			if (size.y < 0)
				size.y = 0;

			button->SetW(size.x);
			button->SetH(size.y);
		}

		if (ImGui::DragFloat2("Button Pos", (float*)&pos, 0.005f, 0.0f, 0.0f, "%.3f", NULL))
		{
			button->SetX(pos.x);
			button->SetY(pos.y);
		}

		ImGui::ColorEdit3("Idle Diffuse Color", (float*)&button->idle, ImGuiColorEditFlags_NoAlpha);
		ImGui::SliderFloat("Idle Diffuse Alpha", (float*)&button->idle.a, 0.0f, 1.0f, "%.3f");

		ImGui::ColorEdit3("Hovered Diffuse Color", (float*)&button->hovered, ImGuiColorEditFlags_NoAlpha);
		ImGui::SliderFloat("Hovered Diffuse Alpha", (float*)&button->hovered.a, 0.0f, 1.0f, "%.3f");

		ImGui::ColorEdit3("Pressed Diffuse Color", (float*)&button->pressed, ImGuiColorEditFlags_NoAlpha);
		ImGui::SliderFloat("Pressed Diffuse Alpha", (float*)&button->pressed.a, 0.0f, 1.0f, "%.3f");

		static float copyedColours[12];

		if (ImGui::Button("Copy Colors"))
		{
			memcpy(copyedColours, (float*)&button->idle, 4*sizeof(float));
			memcpy(copyedColours+4, (float*)&button->hovered, 4 * sizeof(float));
			memcpy(copyedColours+8, (float*)&button->pressed, 4 * sizeof(float));
		}
		ImGui::SameLine();
		if (ImGui::Button("Paste Colors"))
		{
			memcpy((float*)&button->idle, copyedColours, 4 * sizeof(float));
			memcpy((float*)&button->hovered, copyedColours+4, 4 * sizeof(float));
			memcpy((float*)&button->pressed, copyedColours+8, 4 * sizeof(float));
		}

		if (ImGui::DragInt4("pixel TexCoord (x,y,w,h)", button->pixelCoord))
			button->textCoord = button->GetTexturePosition(button->pixelCoord[0], button->pixelCoord[1], button->pixelCoord[2], button->pixelCoord[3]);

		if (!show)
		{
			componentToDelete = button;
			showDeleteComponentPopup = true;
		}

		ImGui::Separator();
	}
}

//void E_Inspector::DrawPlayerControllerComponent(C_PlayerController* cController)
//{
//	bool show = true;
//	if (ImGui::CollapsingHeader("Player Controller", &show, ImGuiTreeNodeFlags_Leaf))
//	{
//		bool isActive = cController->IsActive();
//		if (ImGui::Checkbox("Controller Is Active", &isActive))
//			cController->SetIsActive(isActive);
//
//		ImGui::SameLine(ImGui::GetWindowWidth() * 0.69f);
//
//		uint state = (uint)cController->state;
//		ImGui::Text("State:"); ImGui::SameLine(); ImGui::TextColored(&Yellow, "{ %u }", state);
//
//		ImGui::Separator();
//		if (ImGui::TreeNodeEx("Character"))
//		{
//			float speed = cController->Speed();
//			if (ImGui::InputFloat("Speed", &speed, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
//				cController->SetSpeed(speed);
//
//			ImGui::Separator();
//
//			uint state = (uint)cController->state;
//			ImGui::Text("Player State:"); ImGui::SameLine(); ImGui::TextColored(Yellow.C_Array(), "{ %u }", state);
//
//			ImGui::TreePop();
//		}
//
//		if (ImGui::TreeNodeEx("Weapon"))
//		{
//			float bulletSpeed = cController->BulletSpeed();
//			if (ImGui::InputFloat("Bullet Speed", &bulletSpeed, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
//				cController->SetBulletSpeed(bulletSpeed);
//
//			float fireRate = cController->FireRate();
//			if (ImGui::InputFloat("Fire Rate", &fireRate, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
//				cController->SetFireRate(fireRate);
//
//			int ammo = cController->CurrentAmmo();
//			if (ImGui::InputInt("Ammo", &ammo, 1, 10, ImGuiInputTextFlags_EnterReturnsTrue))
//				cController->SetCurrentAmmo(ammo);
//
//			int maxAmmo = cController->MaxAmmo();
//			if (ImGui::InputInt("Max Ammo", &maxAmmo, 1, 10, ImGuiInputTextFlags_EnterReturnsTrue))
//				cController->SetMaxAmmo(maxAmmo);
//
//			bool automatic = cController->IsAutomatic();
//			if (ImGui::Checkbox("Automatic", &automatic))
//				cController->SetAutomatic(automatic);
//
//			ImGui::Separator();
//
//			ImGui::TreePop();
//		}
//
//		if (ImGui::TreeNodeEx("Dash"))
//		{
//			float dashSpeed = cController->DashSpeed();
//			if (ImGui::InputFloat("Dash Speed", &dashSpeed, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
//				cController->SetDashSpeed(dashSpeed);
//
//			float dashTime = cController->DashTime();
//			if (ImGui::InputFloat("Dash Time", &dashTime, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
//				cController->SetDashTime(dashTime);
//
//			float dashColdown = cController->DashColdown();
//			if (ImGui::InputFloat("Dash Coldown", &dashColdown, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
//				cController->SetDashColdown(dashColdown);
//
//			ImGui::TreePop();
//		}
//
//		if (!show)
//		{
//			componentToDelete = cController;
//			showDeleteComponentPopup = true;
//		}
//
//		ImGui::Separator();
//	}
//	return;
//}
//
//void E_Inspector::DrawBulletBehaviorComponent(C_BulletBehavior* cBehavior)
//{
//	bool show = true;
//	if (ImGui::CollapsingHeader("Bullet Bahavior", &show, ImGuiTreeNodeFlags_Leaf))
//	{
//		bool isActive = cBehavior->IsActive();
//		if (ImGui::Checkbox("Bullet Is Active", &isActive))
//			cBehavior->SetIsActive(isActive);
//
//		ImGui::Separator();
//
//		float autodestruct = cBehavior->GetAutodestruct();
//		if (ImGui::InputFloat("Autodestruction", &autodestruct, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
//			cBehavior->SetAutodestruct(autodestruct);
//
//		if (!show)
//		{
//			componentToDelete = cBehavior;
//			showDeleteComponentPopup = true;
//		}
//
//		ImGui::Separator();
//	}
//	return;
//}
//
//void E_Inspector::DrawPropBehaviorComponent(C_PropBehavior* cBehavior)
//{
//	bool show = true;
//	if (ImGui::CollapsingHeader("Prop Bahavior", &show, ImGuiTreeNodeFlags_Leaf))
//	{
//		bool isActive = cBehavior->IsActive();
//		if (ImGui::Checkbox("Prop Is Active", &isActive))
//			cBehavior->SetIsActive(isActive);
//
//		if (!show)
//		{
//			componentToDelete = cBehavior;
//			showDeleteComponentPopup = true;
//		}
//
//		ImGui::Separator();
//	}
//	return;
//}
//
//void E_Inspector::DrawCameraBehaviorComponent(C_CameraBehavior* cBehavior)
//{
//	bool show = true;
//	if (ImGui::CollapsingHeader("Camera Bahavior", &show, ImGuiTreeNodeFlags_Leaf))
//	{
//		bool isActive = cBehavior->IsActive();
//		if (ImGui::Checkbox("Camera Behavior Is Active", &isActive))
//			cBehavior->SetIsActive(isActive);
//
//		ImGui::Separator();
//
//		float3 offset = cBehavior->GetOffset();
//		float o[3] = { offset.x, offset.y, offset.z };
//		if (ImGui::InputFloat3("Offset", o, 4, ImGuiInputTextFlags_EnterReturnsTrue))
//			cBehavior->SetOffset(float3(o[0], o[1], o[2]));
//
//		if (!show)
//		{
//			componentToDelete = cBehavior;
//			showDeleteComponentPopup = true;
//		}
//
//		ImGui::Separator();
//	}
//	return;
//}
//
//void E_Inspector::DrawGateBehaviorComponent(C_GateBehavior* cBehavior)
//{
//	bool show = true;
//	if (ImGui::CollapsingHeader("Gate Bahavior", &show, ImGuiTreeNodeFlags_Leaf))
//	{
//		bool isActive = cBehavior->IsActive();
//		if (ImGui::Checkbox("Gate Is Active", &isActive))
//			cBehavior->SetIsActive(isActive);
//
//		ImGui::Separator();
//
//		if (!show)
//		{
//			componentToDelete = cBehavior;
//			showDeleteComponentPopup = true;
//		}
//
//		ImGui::Separator();
//	}
//	return;
//}

void E_Inspector::DrawAnimator2DComponent(C_2DAnimator* cAnimator)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Animator 2D", &show, ImGuiTreeNodeFlags_Leaf))
	{
		bool isActive = cAnimator->IsActive();
		if (ImGui::Checkbox("Animator 2D is active", &isActive))
			cAnimator->SetIsActive(isActive);

		int k = cAnimator->GetAnimationStepTime();
		if (ImGui::InputInt("Step time",&k));
			cAnimator->SetAnimationStepTime(k);

		static char buffer[64];
		strcpy_s(buffer, cAnimator->GetName(1));
		if (ImGui::InputText("Animation Name", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			cAnimator->ChangeName(buffer,1);
			cAnimator->GetAnimationSprites(buffer,1);
		}

		static char buffer1[64];
		strcpy_s(buffer1, cAnimator->GetName(2));
		if (ImGui::InputText("Aditional Animation Name", buffer1, IM_ARRAYSIZE(buffer1), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			cAnimator->ChangeName(buffer1, 2);
			cAnimator->GetAnimationSprites(buffer1, 2);
		}

		static char buffer2[64];
		strcpy_s(buffer2, cAnimator->GetName(3));
		if (ImGui::InputText("Aditional Animation Name 1", buffer2, IM_ARRAYSIZE(buffer2), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			cAnimator->ChangeName(buffer2, 3);
			cAnimator->GetAnimationSprites(buffer2, 3);
		}

		bool animationOnLoopFromStart = cAnimator->GetAnimationPlayFromStart();
		if (ImGui::Checkbox("Set animation on loop from start:", &animationOnLoopFromStart))
			cAnimator->SetAnimationPlayFromStart(animationOnLoopFromStart);

		if (cAnimator->spritesheet != nullptr)
		{
			ImGui::Text("Spritesheet 1 settings");

			int r = cAnimator->spritesheet->rows;
			int c = cAnimator->spritesheet->columns;
			int a = cAnimator->spritesheet->animationNumber;
			ImGui::InputInt("Spritesheet rows", &r);
			ImGui::InputInt("Spritesheet columns", &c);
			ImGui::InputInt("Number of animations", &a);
			cAnimator->spritesheet->rows = r;
			cAnimator->spritesheet->columns = c;
			cAnimator->spritesheet->animationNumber = a;
		}

		if (cAnimator->spritesheet2 != nullptr)
		{
		ImGui::Text("Spritesheet 2 settings");

			int rr = cAnimator->spritesheet2->rows;
			int cc = cAnimator->spritesheet2->columns;
			int aa = cAnimator->spritesheet2->animationNumber;
			ImGui::InputInt("Spritesheet2 rows", &rr);
			ImGui::InputInt("Spritesheet2 columns", &cc);
			ImGui::InputInt("Number of animations 2", &aa);
			cAnimator->spritesheet2->rows = rr;
			cAnimator->spritesheet2->columns = cc;
			cAnimator->spritesheet2->animationNumber = aa;
		}

		if (cAnimator->spritesheet3 != nullptr)
		{
			ImGui::Text("Spritesheet 3 settings");

			int rrr = cAnimator->spritesheet3->rows;
			int ccc = cAnimator->spritesheet3->columns;
			int aaa = cAnimator->spritesheet3->animationNumber;
			ImGui::InputInt("Spritesheet3 rows", &rrr);
			ImGui::InputInt("Spritesheet3 columns", &ccc);
			ImGui::InputInt("Number of animations 3", &aaa);
			cAnimator->spritesheet3->rows = rrr;
			cAnimator->spritesheet3->columns = ccc;
			cAnimator->spritesheet3->animationNumber = aaa;
		}





		ImGui::Separator();

		if (!show)
		{
			componentToDelete = cAnimator;
			showDeleteComponentPopup = true;
		}




	}
	return;
}

void E_Inspector::DrawNavMeshAgentComponent(C_NavMeshAgent* cNavMeshAgent)
{
	bool show = true;
	if (ImGui::CollapsingHeader("NavMesh Agent", &show, ImGuiTreeNodeFlags_None))
	{
		DrawBasicSettings((Component*)cNavMeshAgent);

		ImGui::Separator();

		ImGui::Text("WORK IN PROGRESS");

		if (!show)
		{
			componentToDelete = cNavMeshAgent;
			showDeleteComponentPopup = true;
		}
	}
	
	return;
}

void E_Inspector::DrawUICheckboxComponent(C_UI_Checkbox* checkbox)
{
	static bool show = true;
	if (ImGui::CollapsingHeader("CheckBox", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool isActive = checkbox->IsActive();
		if (ImGui::Checkbox("Checkbox Is Active", &isActive)) { checkbox->SetIsActive(isActive); }

		ImGui::Separator();

		// --- RECT ---
		float2 pos = { checkbox->GetRect().x, checkbox->GetRect().y };
		float2 size = { checkbox->GetRect().w, checkbox->GetRect().h };

		//C_Canvas* canvas = button->GetOwner()->parent->GetComponent<C_Canvas>();

		if (ImGui::DragFloat2("Checkbox Size", (float*)&size, 0.005f, 0.0f, 0.0f, "%.3f", NULL))
		{
			if (size.x < 0)
				size.x = 0;
			if (size.y < 0)
				size.y = 0;

			checkbox->SetW(size.x);
			checkbox->SetH(size.y);
		}

		if (ImGui::DragFloat2("Checkbox Pos", (float*)&pos, 0.005f, 0.0f, 0.0f, "%.3f", NULL))
		{
			checkbox->SetX(pos.x);
			checkbox->SetY(pos.y);
		}

		UICheckboxState selected = checkbox->state;
		const char* nameSelected = checkbox->NameFromState(selected);
		if (ImGui::BeginCombo("##Current Draw State", nameSelected, ImGuiComboFlags_PopupAlignLeft))
		{
			if (ImGui::Selectable("UNCHECKED", !strcmp("UNCHECKED", nameSelected)))
				checkbox->state = UICheckboxState::UNCHECKED;
			if (ImGui::Selectable("CHECKED", !strcmp("CHECKED", nameSelected)))
				checkbox->state = UICheckboxState::CHECKED;
			if (ImGui::Selectable("HOVERED_UNCHECKED", !strcmp("HOVERED_UNCHECKED", nameSelected)))
				checkbox->state = UICheckboxState::HOVERED_UNCHECKED;
			if (ImGui::Selectable("HOVERED_CHECKED", !strcmp("HOVERED_CHECKED", nameSelected)))
				checkbox->state = UICheckboxState::HOVERED_CHECKED;
			if (ImGui::Selectable("PRESSED_UNCHECKED", !strcmp("PRESSED_UNCHECKED", nameSelected)))
				checkbox->state = UICheckboxState::PRESSED_UNCHECKED;
			if (ImGui::Selectable("PRESSED_CHECKED", !strcmp("PRESSED_CHECKED", nameSelected)))
				checkbox->state = UICheckboxState::PRESSED_CHECKED;
			ImGui::EndCombo();
		}

		ImGui::Text("pixel TexCoord (x,y,w,h)");
		switch (checkbox->state) 
		{
		case UICheckboxState::UNCHECKED:
			if (ImGui::DragInt4("Unhovered Unchecked", checkbox->pixelCoord))
				checkbox->unhoverUnchecked = checkbox->GetTexturePosition(checkbox->pixelCoord[0], checkbox->pixelCoord[1], checkbox->pixelCoord[2], checkbox->pixelCoord[3]);
			break;
		case UICheckboxState::HOVERED_UNCHECKED:
			if (ImGui::DragInt4("Hovered Unchecked", checkbox->pixelCoord + 4))
				checkbox->hoverUnchecked = checkbox->GetTexturePosition(checkbox->pixelCoord[4], checkbox->pixelCoord[5], checkbox->pixelCoord[6], checkbox->pixelCoord[7]);
			break;
		case UICheckboxState::CHECKED:
			if (ImGui::DragInt4("Unhovered Checked", checkbox->pixelCoord + 8))
				checkbox->unhoverChecked = checkbox->GetTexturePosition(checkbox->pixelCoord[8], checkbox->pixelCoord[9], checkbox->pixelCoord[10], checkbox->pixelCoord[11]);
			break;
		case UICheckboxState::HOVERED_CHECKED:
			if (ImGui::DragInt4("Hovered Checked", checkbox->pixelCoord + 12))
				checkbox->hoverChecked = checkbox->GetTexturePosition(checkbox->pixelCoord[12], checkbox->pixelCoord[13], checkbox->pixelCoord[14], checkbox->pixelCoord[15]);
			break;
		case UICheckboxState::PRESSED_CHECKED_IN:
			if (ImGui::DragInt4("Pressed Checked", checkbox->pixelCoord + 16))
				checkbox->pressedChecked = checkbox->GetTexturePosition(checkbox->pixelCoord[16], checkbox->pixelCoord[17], checkbox->pixelCoord[18], checkbox->pixelCoord[19]);
			break;
		case UICheckboxState::PRESSED_CHECKED:
			if (ImGui::DragInt4("Pressed Checked", checkbox->pixelCoord + 16))
				checkbox->pressedChecked = checkbox->GetTexturePosition(checkbox->pixelCoord[16], checkbox->pixelCoord[17], checkbox->pixelCoord[18], checkbox->pixelCoord[19]);
			break;
		case UICheckboxState::PRESSED_CHECKED_OUT:
			if (ImGui::DragInt4("Pressed Checked", checkbox->pixelCoord + 16))
				checkbox->pressedChecked = checkbox->GetTexturePosition(checkbox->pixelCoord[16], checkbox->pixelCoord[17], checkbox->pixelCoord[18], checkbox->pixelCoord[19]);
			break;
		case UICheckboxState::PRESSED_UNCHECKED_IN:
			if (ImGui::DragInt4("Pressed Unchecked", checkbox->pixelCoord + 20))
				checkbox->pressedUnchecked = checkbox->GetTexturePosition(checkbox->pixelCoord[20], checkbox->pixelCoord[21], checkbox->pixelCoord[22], checkbox->pixelCoord[23]);
			break;
		case UICheckboxState::PRESSED_UNCHECKED:
			if (ImGui::DragInt4("Pressed Unchecked", checkbox->pixelCoord + 20))
				checkbox->pressedUnchecked = checkbox->GetTexturePosition(checkbox->pixelCoord[20], checkbox->pixelCoord[21], checkbox->pixelCoord[22], checkbox->pixelCoord[23]);
			break;
		case UICheckboxState::PRESSED_UNCHECKED_OUT:
			if (ImGui::DragInt4("Pressed Unchecked", checkbox->pixelCoord + 20))
				checkbox->pressedUnchecked = checkbox->GetTexturePosition(checkbox->pixelCoord[20], checkbox->pixelCoord[21], checkbox->pixelCoord[22], checkbox->pixelCoord[23]);
			break;
		}

		if (!show)
		{
			componentToDelete = checkbox;
			showDeleteComponentPopup = true;
		}

		ImGui::Separator();
	}
}

// --- DRAW COMPONENT UTILITY METHODS ---
void E_Inspector::AddComponentCombo(GameObject* selectedGameObject)
{
	ImGui::Combo("##", &componentType, "Add Component\0Transform\0Mesh\0Material\0Light\0Camera\0Animator\0Animation\0RigidBody\0Box Collider\0Sphere Collider\0Capsule Collider\0Particle System\0Canvas\0Audio Source\0Audio Listener\0Player Controller\0Bullet Behavior\0Prop Behavior\0Camera Behavior\0Gate Behavior\0UI Image\0UI Text\0UI Button\0Script\0Animator 2D\0NavMesh Agent\0UI Checkbox");

	ImGui::SameLine();

	if ((ImGui::Button("ADD")))
	{ 
		switch (componentType)
		{
		case (int)ComponentType::NONE: break;
		case (int)ComponentType::UI_IMAGE: AddUIComponent(selectedGameObject, ComponentType::UI_IMAGE); break;
		case (int)ComponentType::UI_TEXT: AddUIComponent(selectedGameObject, ComponentType::UI_TEXT); break;
		case (int)ComponentType::UI_BUTTON: AddUIComponent(selectedGameObject, ComponentType::UI_BUTTON); break;
		case (int)ComponentType::UI_CHECKBOX: AddUIComponent(selectedGameObject, ComponentType::UI_CHECKBOX); break;
		default: selectedGameObject->CreateComponent((ComponentType)componentType); break;
		}
	}
}

void E_Inspector::AddUIComponent(GameObject* selectedGameObject, ComponentType type)
{
	// Option 1: selectedGameObject has a canvas
	if (selectedGameObject->GetComponent<C_Canvas>() != nullptr)
	{
		GameObject* newGO;
		newGO = App->scene->CreateGameObject("New Ui", selectedGameObject);
		newGO->CreateComponent(type);
	}
	// Option 2: selectedGameObject's parent has a canvas
	else if (selectedGameObject->parent->GetComponent<C_Canvas>() != nullptr)
	{
		selectedGameObject->SetName("New Ui");
		selectedGameObject->CreateComponent(type);
	}
	// Option 3: need to crete a canvas
	else
	{
		selectedGameObject->SetName("Canvas");
		selectedGameObject->CreateComponent(ComponentType::CANVAS);

		GameObject* newImage = App->scene->CreateGameObject("New Ui", selectedGameObject);
		newImage->CreateComponent(type);
	}
}

void E_Inspector::DeleteComponentPopup(GameObject* selectedGameObject)
{
	std::string title	=	"Delete ";																// Generating the specific string for the Popup title.
	title				+=	componentToDelete->GetNameFromType();									// The string will be specific to the component to delete.
	title				+=	" Component?";															// -------------------------------------------------------
	
	ImGui::OpenPopup(title.c_str());
	
	bool show = true;																				// Dummy bool to close the popup without having to click the "CONFIRM" or "CANCEL" Buttons.
	if (ImGui::BeginPopupModal(title.c_str(), &show))
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.25f));
		if (ImGui::Button("CONFIRM"))																// CONFIRM Button. Will delete the component to delete.
		{
			selectedGameObject->DeleteComponent(componentToDelete);
			
			componentToDelete				= nullptr;
			showDeleteComponentPopup		= false;

			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor();

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.25f));
		if (ImGui::Button("CANCEL"))																// CANCEL Button. Will close the Popup.
		{
			componentToDelete				= nullptr;
			showDeleteComponentPopup		= false;
			
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor();

		if (!show)																					// Popup cross. Will close the Popup. UX.
		{
			componentToDelete				= nullptr;
			showDeleteComponentPopup		= false;
			
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void E_Inspector::DrawBasicSettings(Component* component, const char* state)
{
	std::string label = "";
	switch (component->GetType())																			// Later give the label a unique ID with "...##UID".
	{
	case ComponentType::NONE:				{ label = "[NONE] is active"; }				break;
	case ComponentType::TRANSFORM:			{ label = "Transform is active"; }			break;
	case ComponentType::MESH:				{ label = "Mesh is active"; }				break;
	case ComponentType::MATERIAL:			{ label = "Material is active"; }			break;
	case ComponentType::LIGHT:				{ label = "Light is active"; }				break;
	case ComponentType::CAMERA:				{ label = "Camera is active"; }				break;
	case ComponentType::ANIMATOR:			{ label = "Animator is active"; }			break;
	case ComponentType::ANIMATION:			{ label = "Animation is active"; }			break;
	case ComponentType::RIGIDBODY:			{ label = "RigidBody is active"; }			break;
	case ComponentType::BOX_COLLIDER:		{ label = "Box Collider is active"; }		break;
	case ComponentType::SPHERE_COLLIDER:	{ label = "Sphere Collider is active"; }	break;
	case ComponentType::CAPSULE_COLLIDER:	{ label = "Capsule Collider is active"; }	break;
	case ComponentType::PARTICLE_SYSTEM:	{ label = "Particles is active"; }			break;
	case ComponentType::CANVAS:				{ label = "Canvas is active"; }				break;
	case ComponentType::AUDIOSOURCE:		{ label = "Audio Source is active";}		break;
	case ComponentType::AUDIOLISTENER:		{ label = "Audio Listener is active"; }		break;
	case ComponentType::UI_IMAGE:			{ label = "UI Image is active"; }			break;
	case ComponentType::UI_TEXT:			{ label = "UI Text is active"; }			break;
	case ComponentType::UI_BUTTON: 			{ label = "UI Button is active"; }			break;
	case ComponentType::SCRIPT:				{ label = "Script is active"; }				break;
	case ComponentType::ANIMATOR2D:			{ label = "Animator 2D is active"; }		break;
	case ComponentType::NAVMESH_AGENT:		{ label = "NavMesh Agent is active"; }		break;
	case ComponentType::PLAYER_CONTROLLER:	{ label = "Player Controller is active"; }	break;
	case ComponentType::BULLET_BEHAVIOR:	{ label = "Bullet Behavior is active"; }	break;
	case ComponentType::PROP_BEHAVIOR:		{ label = "Prop Behavior is active"; }		break;
	case ComponentType::CAMERA_BEHAVIOR:	{ label = "Camera Behavior is active"; }	break;
	case ComponentType::GATE_BEHAVIOR:		{ label = "Gate Behavior is active"; }		break;
	}
	
	bool isActive = component->IsActive();
	if (ImGui::Checkbox(label.c_str(), &isActive)) { component->SetIsActive(isActive); }

	if (state != nullptr)
	{
		ImGui::SameLine(ImGui::GetWindowWidth() * 0.69f); 
		ImGui::Text("State:"); ImGui::SameLine(); ImGui::TextColored(&Yellow, "{ %s }", state);
	}

	ImGui::Separator();
}

void E_Inspector::DisplayMeshSelector(C_Mesh* cMesh, std::map<std::string, ResourceBase>& meshBases)
{
	if (ImGui::BeginCombo("Select Mesh", cMesh->GetMeshFile(), ImGuiComboFlags_None))
	{
		if (meshBases.empty())																									// Convoluted and overengineered, but it works :).
		{
			std::vector<ResourceBase> rBases;
			App->resourceManager->GetResourceBases<R_Mesh>(rBases);

			for (auto base = rBases.cbegin(); base != rBases.cend(); ++base)
			{
				meshBases.emplace((*base).assetsFile, *base);																	// std::map will sort the bases by Alphabetical Order.
			}

			rBases.clear();
		}
			
		for (auto mesh = meshBases.cbegin(); mesh != meshBases.cend(); ++mesh)
		{
			if (ImGui::Selectable(mesh->first.c_str(), (mesh->first == cMesh->GetMeshFile())))
			{	
				bool success = App->resourceManager->AllocateResource(mesh->second.UID, mesh->second.assetsPath.c_str());
				if (success) { cMesh->SetMesh((R_Mesh*)App->resourceManager->RequestResource(mesh->second.UID)); }

				meshBases.clear();

				break;
			}
		}
		
		ImGui::EndCombo();
	}
}

void E_Inspector::DisplayMaterial(C_Material* cMaterial)
{
	Color color = cMaterial->GetMaterialColour();
	static std::map<std::string, ResourceBase> materialBases;

	ImGui::TextColored(Green.C_Array(), "Material:");
	
	// --- MATERIAL SELECTOR ---
	ImGui::Separator();
	
	DisplayMaterialSelector(cMaterial, materialBases);

	ImGui::Separator();

	// --- MATERIAL COLOR & ALPHA ---
	ImGui::TextColored(Cyan.C_Array(), "Material Data:");

	if (ImGui::ColorEdit3("Diffuse Color", (float*)&color, ImGuiColorEditFlags_NoAlpha))	{ cMaterial->SetMaterialColour(color); }
	if (ImGui::SliderFloat("Diffuse Alpha", (float*)&color.a, 0.0f, 1.0f, "%.3f"))			{ cMaterial->SetMaterialColour(color); }
	
	if (ImGui::Button("Save Material"))														{ /*App->resourceManager->SaveResourceToLibrary((Resource*)cMaterial->GetMaterial());*/ }
}

void E_Inspector::DisplayMaterialSelector(C_Material* cMaterial, std::map<std::string, ResourceBase>& materialBases)
{
	if (ImGui::BeginCombo("Select Material", "[SELECT MATERIAL]"))
	{
		ImGui::EndCombo();
	}
}

void E_Inspector::DisplayShader(C_Material* cMaterial)
{
	static std::map<std::string, ResourceBase> shaderBases;
	
	ImGui::TextColored(Green.C_Array(), "Shader:");

	// --- SHADER SELECTOR ---
	ImGui::Separator();
	
	DisplayShaderSelector(cMaterial, shaderBases);

	if (ImGui::Button("Edit Shader")) { CallTextEditor(cMaterial); }
	
	ImGui::Separator();

	// --- SHADER DATA ---
	R_Shader* shader = cMaterial->GetShader();
	if (shader == nullptr)
	{
		return;
	}

	if (!shader->uniforms.empty())
	{
		if (ImGui::TreeNodeEx("Shader Data:"))
		{
			for (uint i = 0; i < shader->uniforms.size(); i++)
			{
				switch (shader->uniforms[i].uniformType)
				{
				case UniformType::INT:			ImGui::DragInt(shader->uniforms[i].name.c_str(), &shader->uniforms[i].integer, 0.02f, 0.0f, 0.0f, "%.2f");							break;
				case UniformType::FLOAT:		ImGui::DragFloat(shader->uniforms[i].name.c_str(), &shader->uniforms[i].floatNumber, 0.02f, 0.0f, 0.0f, "%.2f");					break;
				case UniformType::INT_VEC2:		ImGui::DragInt2(shader->uniforms[i].name.c_str(), (int*)&shader->uniforms[i].vec2, 0.02f, 0.0f, 0.0f, "%.2f");						break;
				case UniformType::INT_VEC3:		ImGui::DragInt3(shader->uniforms[i].name.c_str(), (int*)&shader->uniforms[i].vec3, 0.02f, 0.0f, 0.0f, "%.2f");						break;
				case UniformType::INT_VEC4:		ImGui::DragInt4(shader->uniforms[i].name.c_str(), (int*)&shader->uniforms[i].vec4, 0.02f, 0.0f, 0.0f, "%.2f");						break;
				case UniformType::FLOAT_VEC2:	ImGui::DragFloat2(shader->uniforms[i].name.c_str(), (float*)&shader->uniforms[i].vec2, 0.02f, 0.0f, 0.0f, "%.2f");					break;
				case UniformType::FLOAT_VEC3:	ImGui::DragFloat3(shader->uniforms[i].name.c_str(), (float*)&shader->uniforms[i].vec3, 0.02f, 0.0f, 0.0f, "%.2f");					break;
				case UniformType::FLOAT_VEC4:	ImGui::DragFloat4(shader->uniforms[i].name.c_str(), (float*)&shader->uniforms[i].vec4, 0.02f, 0.0f, 0.0f, "%.2f");					break;
				case UniformType::MATRIX4:		ImGui::DragFloat4(shader->uniforms[i].name.c_str(), shader->uniforms[i].matrix4.ToEulerXYZ().ptr(), 0.02f, 0.0f, 0.0f, "%.2f");		break;
				}
			}

			ImGui::TreePop();
		}
		
		if (ImGui::Button("Save Uniforms"))
		{
			App->resourceManager->SaveResourceToLibrary(cMaterial->GetShader());
		}
	}
}

void E_Inspector::DisplayShaderSelector(C_Material* cMaterial, std::map<std::string, ResourceBase>& shaderBases)
{
	const char* currentShaderName = (cMaterial->GetShader() != nullptr) ? cMaterial->GetShader()->GetAssetsFile() : "[NONE]";
	
	if (ImGui::BeginCombo("Select Shader", currentShaderName, ImGuiComboFlags_PopupAlignLeft))
	{
		if (shaderBases.empty())
		{
			std::vector<ResourceBase> rBases;
			App->resourceManager->GetResourceBases<R_Shader>(rBases);

			for (auto base = rBases.cbegin(); base != rBases.cend(); ++base)
			{
				shaderBases.emplace((*base).assetsFile.c_str(), *base);
			}

			rBases.clear();
		}
		
		for (auto shader = shaderBases.cbegin(); shader != shaderBases.cend(); ++shader)
		{
			if (ImGui::Selectable(shader->first.c_str(), (shader->first == cMaterial->GetShader()->GetAssetsFile())))
			{
				cMaterial->SetShader(App->resourceManager->GetResource<R_Shader>(shader->second.assetsPath.c_str()));

				shaderBases.clear();

				break;
			}
		}

		ImGui::EndCombo();
	}
}

void E_Inspector::DisplayTexture(C_Material* cMaterial)
{
	static std::map<std::string, ResourceBase> textureBases;
	static int mapToDisplay = 0;
	bool useCheckeredTex	= cMaterial->UseDefaultTexture();
	
	ImGui::TextColored(Green.C_Array(), "Texture:");

	// --- TEXTURE SELECTOR ---
	ImGui::Separator();

	DisplayTextureSelector(cMaterial, textureBases);

	ImGui::Separator();

	// --- TEXTURE DATA ---
	DisplayTextureData(cMaterial);

	ImGui::Separator();

	// --- TEXTURE MAPS ---
	ImGui::TextColored(Cyan.C_Array(), "Maps:");

	if (ImGui::Combo("Textures(WIP)", &mapToDisplay, "Diffuse\0Specular\0Ambient\0Height\0Normal\0"))	{ LOG("[SCENE] Changed to map %d", mapToDisplay); }
	if (ImGui::Checkbox("Use Default Texture", &useCheckeredTex))										{ cMaterial->SetUseDefaultTexture(useCheckeredTex); }

	// --- TEXTURE DISPLAY ---
	TextureImageDisplay(cMaterial);
}

void E_Inspector::DisplayTextureSelector(C_Material* cMaterial, std::map<std::string, ResourceBase>& textureBases)
{
	if (ImGui::BeginCombo("Select Texture", cMaterial->GetTextureFile(), ImGuiComboFlags_None))
	{
		if (textureBases.empty())
		{
			std::vector<ResourceBase> rBases;
			App->resourceManager->GetResourceBases<R_Texture>(rBases);

			for (auto base = rBases.cbegin(); base != rBases.cend(); ++base)
			{
				textureBases.emplace((*base).assetsFile.c_str(), *base);
			}

			rBases.clear();
		}

		for (auto texture = textureBases.cbegin(); texture != textureBases.cend(); ++texture)
		{
			if (ImGui::Selectable(texture->first.c_str(), (texture->first == cMaterial->GetTextureFile())))
			{
				bool success = App->resourceManager->AllocateResource(texture->second.UID, texture->second.assetsPath.c_str());
				cMaterial->SetTexture((R_Texture*)App->resourceManager->RequestResource(texture->second.UID));

				textureBases.clear();

				break;
			}
		}

		ImGui::EndCombo();
	}
}

void E_Inspector::DisplayTextureData(C_Material* cMaterial)
{
	ImGui::TextColored(Cyan.C_Array(), "Texture Data:");

	static uint id				= 0;
	static uint width			= 0;
	static uint height			= 0;
	static uint depth			= 0;
	static uint bpp				= 0;
	static uint size			= 0;
	static std::string format	= "[NONE]";
	static bool compressed		= 0;

	cMaterial->GetTextureInfo(id, width, height, depth, bpp, size, format, compressed);

	ImGui::Text("Texture ID:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%u",			id);
	ImGui::Text("Width:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "     %upx",	width);
	ImGui::Text("Height:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %upx",	height);
	ImGui::Text("Depth:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "     %u",	depth);
	ImGui::Text("Bpp:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "       %uB", bpp);
	ImGui::Text("Size:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "      %uB",	size);
	ImGui::Text("Format:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %s",		format.c_str());
	ImGui::Text("Compressed:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s",			compressed ? "True" : "False");
}

void E_Inspector::TextureImageDisplay(C_Material* cMaterial)
{
	ImTextureID texId		= 0;
	ImVec2 displaySize		= { ImGui::GetWindowWidth() * 0.925f , ImGui::GetWindowWidth() * 0.925f };		// Display Size will be 7.5% smaller than the Window Width.
	ImVec4 tint				= { 1.0f, 1.0f, 1.0f, 1.0f };
	ImVec4 borderColor		= { 0.0f, 1.0f, 0.0f, 1.0f };

	if (cMaterial->UseDefaultTexture())
	{
		texId = (ImTextureID)EngineApp->renderer->GetDebugTextureID();
		//tex_id = (ImTextureID)EngineApp->renderer->GetSceneRenderTexture();
	}
	else
	{
		texId = (ImTextureID)cMaterial->GetTextureID();
	}

	if (texId != 0)
	{
		ImGui::TextColored(Cyan.C_Array(), "Texture Display:");

		ImGui::Spacing();

		ImGui::Image(texId, displaySize, ImVec2(1.0f, 0.0f), ImVec2(0.0f, 1.0f), tint, borderColor);			// ImGui has access to OpenGL's buffers, so only the Texture Id is required.
	}
}

void E_Inspector::DisplayAnimatorSettings(C_Animator* cAnimator)
{	
	// --- ANIMATOR CONTROLS
	DisplayAnimatorControls(cAnimator);

	// --- ANIMATOR SETTINGS
	
	ImGui::Separator();
	ImGui::Separator();

	DisplayTrackViewer(cAnimator);
}

void E_Inspector::DisplayAnimatorControls(C_Animator* cAnimator)
{
	// --- ANIMATOR VARIABLES
	float speed				= cAnimator->GetPlaybackSpeed();
	bool interpolate		= /*cAnimator->GetInterpolate()*/ false;
	bool cameraCulling		= cAnimator->GetCameraCulling();
	bool showBones			= cAnimator->GetShowBones();
	
	ImGui::TextColored(Green.C_Array(), "Animator Controls");

	if (ImGui::SliderFloat("Playback Speed", &speed, 0.1f, 10.0f, "X %.3f", ImGuiSliderFlags_None)) { cAnimator->SetPlaybackSpeed(speed); }

	if (ImGui::Checkbox("Interpolate", &interpolate))		{ /*cAnimator->SetInterpolate(interpolate);*/ }
	if (ImGui::Checkbox("Camera Culling", &cameraCulling))	{ cAnimator->SetCameraCulling(cameraCulling); }
	if (ImGui::Checkbox("Show Bones", &showBones))			{ cAnimator->SetShowBones(showBones); }
	
	if (ImGui::Button("Play All"))	{ cAnimator->Play(true); }	ImGui::SameLine();
	if (ImGui::Button("Pause All"))	{ cAnimator->Pause(); }		ImGui::SameLine();
	if (ImGui::Button("Step All"))	{ cAnimator->Step(); }		ImGui::SameLine();
	if (ImGui::Button("Stop All"))	{ cAnimator->Stop(); }

	ImGui::Separator();
}

void E_Inspector::DisplayTrackViewer(C_Animator* cAnimator)
{
	// --- TRACK VIEWER VARIABLES
	std::map<std::string, AnimatorTrack>* tracks	= cAnimator->GetTracksAsPtr();
	std::map<std::string, AnimatorClip>* clips		= cAnimator->GetClipsAsPtr();
	
	ImGui::TextColored(Cyan.C_Array(), "Track Viewer");
	
	ImGui::BeginChild("Track Viewer Child", ImVec2(0.0f, 269.0f), true, ImGuiWindowFlags_HorizontalScrollbar);

	for (auto trackItem = tracks->begin(); trackItem != tracks->end(); ++trackItem)
	{	
		if (ImGui::TreeNodeEx(trackItem->first.c_str(), ImGuiTreeNodeFlags_None))
		{
			AnimatorTrack* track	= &trackItem->second;
			AnimatorClip* clip		= track->GetCurrentClip();
			AnimatorClip* blendClip = track->GetBlendingClip();
			GameObject* rootBone	= track->GetRootBone();
			float trackSpeed		= track->GetTrackSpeed();
			bool interpolate		= track->GetInterpolate();

			ImGui::BeginChild(trackItem->first.c_str(), ImVec2(0.0f, 202.5f), true, ImGuiWindowFlags_HorizontalScrollbar);

			ImGui::TextColored(Cyan.C_Array(), "Track Controls:");

			// --- Select Clip Combo
			if (ImGui::BeginCombo("Select Clip", ((clip != nullptr) ? clip->GetName() : "[SELECT CLIP]"), ImGuiComboFlags_None))
			{
				for (auto clipItem = clips->begin(); clipItem != clips->end(); ++clipItem)
				{
					if (ImGui::Selectable(clipItem->second.GetName(), (&clipItem->second == clip), ImGuiSelectableFlags_None))
					{
						if (clipItem->second.GetAnimation() == nullptr)
							continue;

						cAnimator->SetTrackWithClip(track, &clipItem->second);

						break;
					}
				}

				ImGui::EndCombo();
			}

			if (ImGui::Button("Play"))					{ cAnimator->PlayClip(track, clip, 0u); }	ImGui::SameLine();
			if (ImGui::Button("Pause"))					{ track->Pause(); }							ImGui::SameLine();
			if (ImGui::Button("Step"))					{ track->Step(); }							ImGui::SameLine();
			if (ImGui::Button("Stop"))					{ track->Stop(); }

			if (ImGui::TreeNodeEx("Advanced Controls", ImGuiTreeNodeFlags_None))
			{
				if (ImGui::BeginCombo("Root Bone", ((rootBone != nullptr) ? rootBone->GetName() : "SELECT ROOT BONE"), ImGuiComboFlags_None))
				{	
					std::vector<GameObject*>* bones = cAnimator->GetBonesAsPtr();
					
					for (uint i = 0; i < bones->size(); ++i)
					{
						GameObject* bone = bones->at(i);
						if (bone == nullptr)
							continue;

						if (ImGui::Selectable(bone->GetName(), (bone == rootBone), ImGuiSelectableFlags_None))
						{
							//rootBone = bone;
							track->SetRootBone(bone);
						}
					}

					ImGui::EndCombo();
				}

				if (ImGui::SliderFloat("Speed", &trackSpeed, 0.1f, 10.0f, "x%.2f")) { track->SetTrackSpeed(trackSpeed); }
				if (ImGui::Checkbox("Interpolate", &interpolate))					{ track->SetInterpolate(interpolate); }

				if (ImGui::Button("Previous Keyframe"))		{ track->StepToPrevKeyframe(); }	ImGui::SameLine();
				if (ImGui::Button("Next Keyframe"))			{ track->StepToNextKeyframe(); }

				ImGui::TreePop();
			}
			
			ImGui::Separator();
			ImGui::Separator();

			ImGui::TextColored(Cyan.C_Array(), "Track Stats:"); ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f);
			ImGui::Text("Track State:");						ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "{ %s }",	track->GetTrackStateAsString());

			if (clip != nullptr)
			{
				ImGui::Text("Current Clip:");	ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), " %s",			clip->GetName());
				ImGui::Text("Blending Clip:");	ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "%s",			(blendClip != nullptr) ? blendClip->GetName() : "[NONE]");
				ImGui::Text("Clip Time:");		ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "    %.3fs",	clip->GetClipTime());
				ImGui::Text("Clip Frame:");		ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "   %.3f",		clip->GetClipFrame());
				ImGui::Text("Clip Tick:");		ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "    %u",		clip->GetClipTick());
			}
			else
			{
				ImGui::TextColored(Yellow.C_Array(), "Track has no Assigned Clip!");
			}
			
			//DisplayTrackStats(track, clip);

			ImGui::EndChild();

			ImGui::TreePop();
		}
	}

	ImGui::EndChild();
}

void E_Inspector::DisplayTrackStats(AnimatorTrack* track, AnimatorClip* clip)
{
	if (track == nullptr)
		return;
	
	ImGui::TextColored(Green.C_Array(), "Track Info:");

	ImGui::Text("Name:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "	  	   %s",			track->GetName());
	ImGui::Text("Root Bone:");			ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "	    %s",			track->GetRootBone()->GetName());
	ImGui::Text("Speed:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "	  	  x%.3f",		track->GetTrackSpeed());
	ImGui::Text("Interpolate:");		ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "	  %s",				(track->GetInterpolate()) ? "True" : "False");
	
	ImGui::Separator();

	if (clip == nullptr)
	{
		ImGui::TextColored(Yellow.C_Array(), "This Track has no Assigned Clip!");
		//ImGui::TreePop();
		return;
	}

	// --- ANIMATION STATS
	ImGui::TextColored(Green.C_Array(), "Animation Info");

	// --- REVISE. POSSIBLE NULLPTR
	ImGui::Text("Name:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "             %s",		clip->GetAnimation()->GetName());
	ImGui::Text("Ticks Per Second:");	ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), " %.3f",				clip->GetAnimation()->GetTicksPerSecond());
	ImGui::Text("Duration:");			ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "         %.3f",		clip->GetAnimation()->GetDuration());

	ImGui::Separator();

	// --- CLIP STATS
	ImGui::TextColored(Green.C_Array(), "Clip Info");

	ImGui::Text("Name:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "             %s",		clip->GetName());
	ImGui::Text("Time:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "             %.3f",	clip->GetClipTime());
	ImGui::Text("Frame:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "            %.3f",	clip->GetClipFrame());
	ImGui::Text("Tick:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "		     %u",		clip->GetClipTick());
	ImGui::Text("Range:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "            %u - %u", clip->GetStart(), clip->GetEnd());
	ImGui::Text("Duration:");			ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "         %.3f",		clip->GetDuration());
	ImGui::Text("Speed:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "            x%.3f",	clip->GetSpeed());
	ImGui::Text("Loop:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "             %s",		(clip->IsLooped()) ? "True" : "False");
}

void E_Inspector::DisplayClipManager(C_Animator* cAnimator)
{
	ImGui::Separator();
	ImGui::Separator();

	// --- ANIMATOR CLIP MANAGER
	if (ImGui::BeginTabBar("ClipManagerTabBar", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Clip Creator"))
		{
			ClipCreatorWindow(cAnimator);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Clip Editor"))
		{
			ClipEditorWindow(cAnimator);

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
	
	ImGui::Separator();
	ImGui::Separator();
	
	// -- EXISTING CLIPS
	ImGui::TextColored(Cyan.C_Array(), "Existing Clips");
	
	ImGui::BeginChild("Existing Clips Child", ImVec2(0.0f, 269.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
	
	std::map<std::string, AnimatorClip>* clips = cAnimator->GetClipsAsPtr();
	if (clips != nullptr)
	{
		for (auto clip = clips->cbegin(); clip != clips->cend(); ++clip)
		{
			if (ImGui::TreeNodeEx(clip->first.c_str(), ImGuiTreeNodeFlags_None))
			{
				ImGui::BeginChild("Existing Tracks Child", ImVec2(0.0f, 105.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
				
				ImGui::Text("Name:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "    %s",		clip->second.GetName());
				ImGui::Text("Range:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "   %u - %u",	clip->second.GetStart(), clip->second.GetEnd());
				ImGui::Text("Duration:");			ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "%.3f",		clip->second.GetDuration());
				ImGui::Text("Speed:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "   x%.3f",	clip->second.GetSpeed());
				ImGui::Text("Loop:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "    %s",		(clip->second.IsLooped()) ? "True" : "False");

				ImGui::EndChild();

				ImGui::TreePop();
			}
		}
	}

	ImGui::EndChild();
}

void E_Inspector::ClipCreatorWindow(C_Animator* cAnimator)
{
	// --- NEW CLIP VARIABLES
	std::vector<R_Animation*>* animations	= cAnimator->GetAnimationsAsPtr();
	
	static R_Animation* selectedAnimation	= nullptr;
	static char newClipName[128]			= "Enter Clip Name";
	static int newClipStart					= 0;
	static int newClipEnd					= 0;
	static int newClipMin					= 0;
	static int newClipMax					= 0;
	static float newClipSpeed				= 1.0f;
	static bool loop						= false;

	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::BeginChild("Clip Creator Child", ImVec2(0.0f, 190.0f), true);
	
	// -- CLIP CREATOR
	ImGui::TextColored(Cyan.C_Array(), "Create Clip");
	
	if (ImGui::BeginCombo("Select Animation", ((selectedAnimation != nullptr) ? selectedAnimation->GetName() : "[SELECT ANIMATION]"), ImGuiComboFlags_None))
	{
		for (uint i = 0; i < animations->size(); ++i)
		{
			R_Animation* rAnimation = animations->at(i);
			if (rAnimation == nullptr)
				continue;

			if (ImGui::Selectable(rAnimation->GetName(), (rAnimation == selectedAnimation), ImGuiSelectableFlags_None))
			{
				selectedAnimation	= rAnimation;

				newClipEnd			= selectedAnimation->GetDuration();
				newClipMax			= selectedAnimation->GetDuration();
			}
		}

		ImGui::EndCombo();
	}

	if (selectedAnimation == nullptr)															// End early in case there is no selected animation yet.
	{
		ImGui::TextColored(Yellow.C_Array(), "Select an Animation to continue.");

		ImGui::EndChild();
		ImGui::PopStyleVar();
		
		return;
	}

	ImGui::InputText("Clip Name", newClipName, IM_ARRAYSIZE(newClipName), (ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll));
	ImGui::SliderInt("Clip Start", &newClipStart, newClipMin, newClipMax);
	ImGui::SliderInt("Clip End", &newClipEnd, newClipMin, newClipMax);
	ImGui::SliderFloat("Clip Speed", &newClipSpeed, 0.1f, 10.0f, "X %.3f", 0);
	ImGui::Checkbox("Loop Clip", &loop);
	
	if (newClipEnd > newClipMax)	{ newClipEnd = newClipMax; }
	if (newClipStart > newClipEnd)	{ newClipEnd = newClipStart; };
	
	// --- CREATE CLIP BUTTON
	ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 0.5f });
	if (ImGui::Button("Create")) 
	{ 
		if (selectedAnimation != nullptr)
		{
			if (EngineApp->gameState != GameState::PLAY)
			{
				bool success = cAnimator->AddClip(AnimatorClip(selectedAnimation, newClipName, newClipStart, newClipEnd, newClipSpeed, loop));
				if (success)
				{
					LOG("[SUCCESS] Animator Inspector: Successfully Created Animator Clip { %s }!", newClipName);

					strcpy_s(newClipName, 128, "Enter Clip Name");																// --- Re-setting the New Clip Parameters
					newClipStart	= 0;																						// 
					newClipEnd		= selectedAnimation->GetDuration();															// 
					loop			= false;																					// --------------------------------------
				}
				else
				{
					LOG("[ERROR] Animator Inspector: Could not Create Clip! Error: C_Animator* AddClip() failed.");
				}
			}
			else
			{
				ImGui::SameLine();
				ImGui::TextColored(Red.C_Array(), "Cannot Create Clips While in Game Mode!");
			}
		}
		else
		{
			LOG("[ERROR] Animator Inspector: Could not Create Clip! Error: No R_Animation* was selected!");
		}
	}

	ImGui::PopStyleColor();
	
	ImGui::EndChild();
	ImGui::PopStyleVar();
}

void E_Inspector::ClipEditorWindow(C_Animator* cAnimator)
{
	std::vector<R_Animation*>* animations		= cAnimator->GetAnimationsAsPtr();
	std::map<std::string, AnimatorClip>* clips	= cAnimator->GetClipsAsPtr();

	static R_Animation* selectedAnimation	= nullptr;
	static AnimatorClip* selectedClip		= nullptr;

	static char editedName[128]		= "Edited Clip Name";
	static int editedStart			= 0;
	static int editedEnd			= 0;
	static int editedMin			= 0;
	static int editedMax			= 0;
	static float editedSpeed		= 1.0f;
	static bool editedLoop			= false;

	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::BeginChild("Clip Editor Child", ImVec2(0.0f, 230.0f), true, ImGuiWindowFlags_NoScrollbar);

	ImGui::TextColored(Cyan.C_Array(), "Edit Clip");

	ImGui::Separator();
	ImGui::Separator();

	if (ImGui::BeginCombo("Select Clip", ((selectedClip != nullptr) ? selectedClip->GetName() : "[SELECT CLIP]"), ImGuiComboFlags_None))
	{
		for (auto clip = clips->begin(); clip != clips->end(); ++clip)
		{
			if (ImGui::Selectable(clip->second.GetName(), (&clip->second == selectedClip), ImGuiSelectableFlags_None))
			{
				selectedClip = &clip->second;

				selectedAnimation = (R_Animation*)selectedClip->GetAnimation();
				
				strcpy(editedName, selectedClip->GetName());
				editedStart		= (int)selectedClip->GetStart();
				editedEnd		= (int)selectedClip->GetEnd();
				editedSpeed		= selectedClip->GetSpeed();
				editedLoop		= selectedClip->IsLooped();

				editedMax		= (selectedAnimation != nullptr) ? selectedAnimation->GetDuration() : 0;
			}
		}
		
		ImGui::EndCombo();
	}

	ImGui::Separator();
	ImGui::Separator();

	if (selectedClip == nullptr)																// End early in case there is no selected clip yet.
	{
		ImGui::TextColored(Yellow.C_Array(), "Select a Clip to continue.");
		
		ImGui::EndChild();
		ImGui::PopStyleVar();

		return;
	}

	if (ImGui::BeginCombo("Edit Animation", ((selectedAnimation != nullptr) ? selectedAnimation->GetName() : "[SELECT ANIMATION]"), ImGuiComboFlags_None))
	{
		for (uint i = 0; i < animations->size(); ++i)
		{
			R_Animation* rAnimation = animations->at(i);
			if (rAnimation == nullptr)
				continue;
			
			if (ImGui::Selectable(rAnimation->GetName(), (rAnimation == selectedAnimation), ImGuiSelectableFlags_None))
			{
				selectedAnimation = rAnimation;

				editedEnd = selectedAnimation->GetDuration();
				editedMax = selectedAnimation->GetDuration();
			}
		}

		ImGui::EndCombo();
	}

	if (selectedAnimation == nullptr)															// End early in case there is no selected animation yet.
	{
		ImGui::TextColored(Yellow.C_Array(), "Select an Animation to continue.");

		ImGui::EndChild();
		ImGui::PopStyleVar();

		return;
	}

	ImGui::InputText("Edit Name", editedName, IM_ARRAYSIZE(editedName), (ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll));
	ImGui::SliderInt("Edit Start", &editedStart, editedMin, editedMax);
	ImGui::SliderInt("Edit End", &editedEnd, editedMin, editedMax);
	ImGui::SliderFloat("Edit Speed", &editedSpeed, 0.1f, 10.0f, "X %.3f", 0);
	ImGui::Checkbox("Loop Clip", &editedLoop);

	if (editedEnd > editedMax)		{ editedEnd = editedMax; }
	if (editedStart > editedEnd)	{ editedEnd = editedStart; }

	// --- SAVE & DELETE CLIP BUTTONS
	ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 0.5f });
	if (ImGui::Button("Save"))		
	{ 
		std::string originalClipName = selectedClip->GetName();

		bool success = cAnimator->EditClip(selectedClip->GetName(), AnimatorClip(selectedAnimation, editedName, editedStart, editedEnd, editedSpeed, editedLoop));
		if (success)
		{
			LOG("[SUCCESS] Animator Inspector: Successfully Edited Clip { %s }!", originalClipName.c_str());
			
			selectedClip = (!clips->empty()) ? &clips->begin()->second : nullptr;

			if (selectedClip != nullptr)
			{
				selectedAnimation = (R_Animation*)selectedClip->GetAnimation();

				strcpy_s(editedName, 128, selectedClip->GetName());																					// --- Re-setting the Edit Clip Parameters
				editedStart		= selectedClip->GetStart();																							// 
				editedEnd		= selectedClip->GetEnd();																							//
				editedSpeed		= selectedClip->GetSpeed();																							//
				editedLoop		= selectedClip->IsLooped();																							// --------------------------------------
			}
			else
			{
				selectedAnimation = nullptr;
				
				strcpy_s(editedName, 128, "Edited Clip Name");
				editedStart		= 0;
				editedEnd		= 0;
				editedSpeed		= 1.0f;
				editedLoop		= false;
			}

			clipWasDeleted	= true;
		}
		else
		{
			LOG("[ERROR] Animator Editor Panel: Could not Create Clip! Error: C_Animator* EditClip() failed.");
		}
	}

	ImGui::PopStyleColor();

	ImGui::SameLine(ImGui::GetWindowWidth() * 0.80f);

	ImGui::PushStyleColor(ImGuiCol_Button, LightRed.C_Array());
	if (ImGui::Button("Delete"))	
	{ 
		if (selectedClip != nullptr)
		{
			clipWasDeleted	= true;

			cAnimator->DeleteClip(selectedClip->GetName());
			selectedClip = nullptr;

		}
	}
	ImGui::PopStyleColor();

	ImGui::EndChild();
	ImGui::PopStyleVar();
}

void E_Inspector::DisplayTrackManager(C_Animator* cAnimator)
{
	ImGui::Separator();
	ImGui::Separator();

	if (ImGui::BeginTabBar("TrackManagerTabBar", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("Track Creator"))
		{
			TrackCreatorWindow(cAnimator);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Track Editor"))
		{
			TrackEditorWindow(cAnimator);

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::Separator();
	ImGui::Separator();

	// EXISTING TRACKS
	ImGui::TextColored(Cyan.C_Array(), "Existing Tracks");

	ImGui::BeginChild("Existing Tracks Child", ImVec2(0.0f, 269.0f), true, ImGuiWindowFlags_HorizontalScrollbar);

	std::map<std::string, AnimatorTrack>* tracks = cAnimator->GetTracksAsPtr();
	if (tracks != nullptr)
	{
		for (auto track = tracks->cbegin(); track != tracks->cend(); ++track)
		{
			if (ImGui::TreeNodeEx(track->first.c_str(), ImGuiTreeNodeFlags_None))
			{	
				GameObject* rootBone = track->second.GetRootBone();
				
				ImGui::BeginChild("Existing Tracks Child", ImVec2(0.0f, 82.5f), true, ImGuiWindowFlags_HorizontalScrollbar);

				ImGui::Text("Name:");			ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "       %s",	track->second.GetName());
				ImGui::Text("Root Bone:");		ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "  %s",		(rootBone != nullptr) ? rootBone->GetName() : "[NONE]");
				ImGui::Text("Speed:");			ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "      x%.3f",	track->second.GetTrackSpeed());
				ImGui::Text("Interpolate:");	ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "%s",			(track->second.GetInterpolate()) ? "True" : "False");
				
				ImGui::EndChild();

				ImGui::TreePop();
			}
		}
	}

	ImGui::EndChild();
}

void E_Inspector::TrackCreatorWindow(C_Animator* cAnimator)
{
	// NEW TRACK VARIABLES
	static char trackName[128]				= "Enter Track Name";
	static float trackSpeed					= 1.0f;
	static bool interpolate					= true;
	std::vector<GameObject*>* bones			= cAnimator->GetBonesAsPtr();
	static GameObject* rootBone				= (!bones->empty()) ? bones->at(0) : nullptr;

	ImGuiInputTextFlags inputTxtFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;

	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::BeginChild("Track Creator Child", ImVec2(0.0f, 145.0f), true);

	ImGui::TextColored(Cyan.C_Array(), "Track Creator");

	ImGui::InputText("Track Name", trackName, IM_ARRAYSIZE(trackName), inputTxtFlags);

	if (ImGui::BeginCombo("Track Root Bone", ((rootBone != nullptr) ? rootBone->GetName() : "[NONE]"), ImGuiComboFlags_None))
	{	
		for (uint i = 0; i < bones->size(); ++i)
		{
			GameObject* bone = bones->at(i);
			if (bone == nullptr)
				continue;

			if (ImGui::Selectable(bone->GetName(), (bone == rootBone), ImGuiSelectableFlags_None))
			{
				rootBone = bone;
			}
		}
		
		ImGui::EndCombo();
	}

	ImGui::SliderFloat("Track Speed", &trackSpeed, 0.1f, 10.0f, "X %.3f");
	ImGui::Checkbox("Interpolate", &interpolate);

	ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 0.5f });
	if (ImGui::Button("Create"))
	{
		if (EngineApp->gameState != GameState::PLAY)
		{
			bool success = cAnimator->AddTrack(AnimatorTrack(trackName, rootBone, trackSpeed, interpolate));
			if (success)
			{
				strcpy(trackName, "Enter Track Name");
				rootBone		= (!bones->empty()) ? bones->at(0) : nullptr;
				trackSpeed		= 1.0f;
				interpolate		= true;
			}
			else
			{
				LOG("[ERROR] Animator Inspector: Could not Create Track! Error: C_Animator AddTrack() failed!");
			}
		}
		else
		{
			ImGui::SameLine();
			ImGui::TextColored(Red.C_Array(), "Cannot Create Tracks while in Game Mode!");
		}
	}

	ImGui::PopStyleColor();

	ImGui::EndChild();
	ImGui::PopStyleVar();
}

void E_Inspector::TrackEditorWindow(C_Animator* cAnimator)
{
	// EDITED TRACK VARIABLES
	std::map<std::string, AnimatorTrack>* tracks = cAnimator->GetTracksAsPtr();
	std::vector<GameObject*>* bones				= cAnimator->GetBonesAsPtr();

	static AnimatorTrack* editedTrack	= nullptr;
	static char	trackName[128]			= "Edited Track Name";
	static GameObject* rootBone			= (!bones->empty()) ? bones->at(0) : nullptr;
	static float trackSpeed				= 1.0f;
	static bool interpolate				= true;

	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::BeginChild("Track Editor Child", ImVec2(0.0f, 190.0f), true);

	ImGui::TextColored(Cyan.C_Array(), "Track Editor");
	
	ImGui::Separator();
	ImGui::Separator();
	
	if (ImGui::BeginCombo("Select Track", ((editedTrack != nullptr) ? editedTrack->GetName() : "SELECT TRACK"), ImGuiComboFlags_None))
	{
		for (auto track = tracks->begin(); track != tracks->end(); ++track)
		{
			if (ImGui::Selectable(track->first.c_str(), (&track->second == editedTrack), ImGuiSelectableFlags_None))
			{
				editedTrack = &track->second;

				strcpy(trackName, editedTrack->GetName());
				rootBone		= editedTrack->GetRootBone();
				trackSpeed		= editedTrack->GetTrackSpeed();
				interpolate		= editedTrack->GetInterpolate();
			}
		}
		
		ImGui::EndCombo();
	}

	ImGui::Separator();
	ImGui::Separator();

	ImGui::InputText("Track Name", trackName, IM_ARRAYSIZE(trackName), (ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll));

	if (ImGui::BeginCombo("Track Root Bone", ((rootBone != nullptr) ? rootBone->GetName() : "SELECT ROOT BONE"), ImGuiComboFlags_None))
	{	
		for (uint i = 0; i < bones->size(); ++i)
		{
			GameObject* bone = bones->at(i);
			if (bone == nullptr)
				continue;

			if (ImGui::Selectable(bone->GetName(), (bone == rootBone), ImGuiSelectableFlags_None))
			{
				rootBone = bone;
			}
		}

		ImGui::EndCombo();
	}

	ImGui::SliderFloat("Track Speed", &trackSpeed, 0.1f, 10.0f, "X %.3f");
	ImGui::Checkbox("Interpolate", &interpolate);

	// --- SAVE & DELETE BUTTONS
	ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 1.0f, 0.0f, 0.5f });
	if (ImGui::Button("Save"))
	{
		if (editedTrack != nullptr)
		{
			std::string originalTrackName = editedTrack->GetName();
			
			bool success = cAnimator->EditTrack(editedTrack->GetName(), AnimatorTrack(trackName, rootBone, trackSpeed, interpolate));
			if (success)
			{
				LOG("[SUCCESS] Animator Inspector: Successfully Edited Track { %s }!", originalTrackName.c_str());

				editedTrack = (!tracks->empty()) ? &tracks->begin()->second : nullptr;

				if (editedTrack == nullptr)
				{
					strcpy(trackName, "Edited Track Name");
					rootBone	= nullptr;
					trackSpeed	= 1.0f;
					interpolate = true;
				}
				else
				{
					strcpy(trackName, editedTrack->GetName());
					rootBone	= editedTrack->GetRootBone();
					trackSpeed	= editedTrack->GetTrackSpeed();
					interpolate = editedTrack->GetInterpolate();
				}

				trackWasDeleted		= true;
			}
			else
			{
				LOG("[ERROR] Animator Inspector: Could not Edit Track { %s }! Error: C_Animator EditTrack() Failed.");
			}
		}
	}

	ImGui::PopStyleColor();

	ImGui::SameLine(ImGui::GetWindowWidth() * 0.80f);

	ImGui::PushStyleColor(ImGuiCol_Button, LightRed.C_Array());
	if (ImGui::Button("Delete")) 
	{ 
		if (editedTrack != nullptr)
		{
			trackWasDeleted		= true;
			
			cAnimator->DeleteTrack(editedTrack->GetName());
			editedTrack = nullptr;

		}
	}

	ImGui::PopStyleColor();

	ImGui::EndChild();
	ImGui::PopStyleVar();
}

void E_Inspector::TextEditorWindow()
{
	if (showTextEditorWindow)
	{
		if (showSaveEditorPopup)
		{
			ImGui::OpenPopup("Save Previous File");
			showSaveEditorPopup = false;
		}

		if (ImGui::BeginPopupModal("Save Previous File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Do you want to save changes before closing the editor? \n\n");
			if (ImGui::Button("Save Changes"))
			{
				std::string textToSave = editor.GetText();
				App->fileSystem->Remove(fileToEdit.c_str());
				App->fileSystem->Save(fileToEdit.c_str(), textToSave.c_str(), editor.GetText().size());

				glDetachShader(shaderToRecompile->shaderProgramID, shaderToRecompile->vertexID);
				glDetachShader(shaderToRecompile->shaderProgramID, shaderToRecompile->fragmentID);
				glDeleteProgram(shaderToRecompile->shaderProgramID);

				Importer::Shaders::Import(shaderToRecompile->GetAssetsPath(), shaderToRecompile);

				showSaveEditorPopup		= false;
				showTextEditorWindow	= true;

				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Don't Save"))
			{
				showSaveEditorPopup		= false;
				showTextEditorWindow	= true;

				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (ImGui::Begin("Text Editor", &showTextEditorWindow, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoCollapse))
		{

			//Update
			auto cpos = editor.GetCursorPosition();
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Save"))
					{
						std::string textToSave = editor.GetText();

						App->fileSystem->Remove(fileToEdit.c_str());
						App->fileSystem->Save(fileToEdit.c_str(), textToSave.c_str(), editor.GetText().size());

						glDetachShader(shaderToRecompile->shaderProgramID, shaderToRecompile->vertexID);
						glDetachShader(shaderToRecompile->shaderProgramID, shaderToRecompile->fragmentID);
						glDeleteProgram(shaderToRecompile->shaderProgramID);

						Importer::Shaders::Import(shaderToRecompile->GetAssetsPath(), shaderToRecompile);
					}

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Edit"))
				{
					bool ro = editor.IsReadOnly();
					if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
						editor.SetReadOnly(ro);
					ImGui::Separator();

					if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
						editor.Undo();
					if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
						editor.Redo();

					ImGui::Separator();

					if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
						editor.Copy();
					if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
						editor.Cut();
					if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
						editor.Delete();
					if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
						editor.Paste();

					ImGui::Separator();

					if (ImGui::MenuItem("Select all", nullptr, nullptr))
						editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("View")) //Might not be necessary? 
				{
					if (ImGui::MenuItem("Dark palette"))
						editor.SetPalette(TextEditor::GetDarkPalette());
					if (ImGui::MenuItem("Light palette"))
						editor.SetPalette(TextEditor::GetLightPalette());
					if (ImGui::MenuItem("Retro blue palette"))
						editor.SetPalette(TextEditor::GetRetroBluePalette());
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
				editor.IsOverwrite() ? "Ovr" : "Ins",
				editor.CanUndo() ? "*" : " ",
				editor.GetLanguageDefinition().mName.c_str(), fileToEdit.c_str());

			editor.Render("TextEditor");
			ImGui::End();
		}
	}
}

void E_Inspector::CallTextEditor(C_Material* cMaterial)
{
	//Only Handles GLSL
	TextEditor::LanguageDefinition lang = TextEditor::LanguageDefinition::GLSL();

	fileToEdit = cMaterial->GetShader()->GetAssetsPath();
	editor.SetShowWhitespaces(false);

	std::ifstream t(fileToEdit.c_str());
	if (t.good())
	{
		std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
		editor.SetText(str);
	}

	showSaveEditorPopup = true;
	showTextEditorWindow = true;

	shaderToRecompile = cMaterial->GetShader();
}

void E_Inspector::RigidBodyFilterCombo(C_RigidBody* cRigidBody)
{
	if (ImGui::BeginCombo("Filter", (*cRigidBody->GetFilter()).c_str()))
	{
		const std::vector<std::string>* const filters = App->physics->GetFilters();

		if (ImGui::Selectable("default"))
			cRigidBody->ChangeFilter("default");

		for (uint i = 0; i < filters->size(); i++)
			if (ImGui::Selectable((*filters)[i].c_str()))
				cRigidBody->ChangeFilter((*filters)[i].c_str());

		ImGui::EndCombo();
	}
}

void E_Inspector::DisplayParticleSystemControls(C_ParticleSystem* cParticleSystem)
{
	static char buffer[64];
	
	//TODO PARTICLE SYSTEM
	if (ImGui::Button("New Particle System"))
	{
		cParticleSystem->AddParticleSystem(buffer); //TODO doesn't appear NewParticleSystem() wtf
	}

	ImGui::SameLine();

	// save system
	if (ImGui::Button("Save Particle System"))
	{
		cParticleSystem->SaveParticleSystem(); //TODO doesn't appear NewParticleSystem() wtf
	}

	//combo showing all resources Already exists App->resourceManager->GetAllParticleSystems()
	if (ImGui::BeginCombo("##Particle Systems", cParticleSystem->resource->name.c_str()))
	{
		std::vector<ResourceBase> particleSystems;
		App->resourceManager->GetResourceBases<R_ParticleSystem>(particleSystems);

		

		for (auto it = particleSystems.begin(); it != particleSystems.end(); ++it)
		{
			bool isSelected = (strcmp(cParticleSystem->resource->GetAssetsPath(),(*it).assetsPath.c_str()) == 0);

			if (ImGui::Selectable((*it).assetsPath.c_str(), isSelected))
			{
				cParticleSystem->SetParticleSystem((*it));
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// --- Particle System NAME ---
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.33f);
	//static char buffer[64];
	strcpy_s(buffer, cParticleSystem->resource->name.c_str());
	if (ImGui::InputText("PS Name", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		std::string path = ASSETS_PARTICLESYSTEMS_PATH;
		path += buffer;
		path += PARTICLESYSTEMS_AST_EXTENSION;
		cParticleSystem->resource->SetAssetsPathAndFile(path.c_str(), buffer);
		cParticleSystem->resource->name = buffer;
	}

	ImGui::SameLine();

	bool preview = cParticleSystem->previewEnabled;
	if (ImGui::Checkbox("Preview", &preview))
	{
		cParticleSystem->EnginePreview(preview);
	}

	bool stop = cParticleSystem->stopSpawn;
	if (ImGui::Checkbox("Stop Spawn", &stop))
	{
		cParticleSystem->stopSpawn = stop;
		if (stop == true)
		{
			cParticleSystem->StopSpawn();
		}
		else
		{
			cParticleSystem->ResumeSpawn();
		}
	}

	bool stopDelete = cParticleSystem->tempDelete;
	if (ImGui::Checkbox("Stop and Delete", &stopDelete))
	{
		cParticleSystem->tempDelete = stopDelete;
		cParticleSystem->StopAndDelete();
	}
}

void E_Inspector::DisplayEmitterInstances(C_ParticleSystem* cParticleSystem)
{
	std::string inputTextName;

	static char buffer[32];
	if (ImGui::InputText("Name a new emitter", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		cParticleSystem->resource->AddNewEmitter(buffer);
		cParticleSystem->RefreshEmitterInstances();
	}

	for (uint i = 0; i < cParticleSystem->emitterInstances.size(); i++) //loop emitters
	{
		Emitter* emitter = cParticleSystem->emitterInstances[i]->emitter;

		bool show = true;
		if (ImGui::CollapsingHeader(cParticleSystem->emitterInstances[i]->emitter->name.c_str(), &show, ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.33f);
			static char buffer[64];
			strcpy_s(buffer, emitter->name.c_str());
			inputTextName = "Emitter Name ##" + std::to_string(i);
			if (ImGui::InputText(inputTextName.c_str(), buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				emitter->name = buffer;
			}

			inputTextName = "Particle Number - " + emitter->name;
			int particleNumber = emitter->maxParticleCount;
			if (ImGui::InputInt(inputTextName.c_str(), &particleNumber, 1, 10, ImGuiInputTextFlags_EnterReturnsTrue))
			{ 
				emitter->SetParticleCount(particleNumber); 
				cParticleSystem->RefreshEmitterInstances();
			}

			R_Texture* current = emitter->emitterTexture;
			//combo showing all resources Already exists App->resourceManager->GetAllParticleSystems()
			inputTextName = "Particle Texture - " + emitter->name;
			if (ImGui::BeginCombo(inputTextName.c_str(), emitter->emitterTexture == nullptr ? "No Texture" : emitter->emitterTexture->GetAssetsFile()))
			{
				std::vector<ResourceBase> textures;
				App->resourceManager->GetResourceBases<R_Texture>(textures);

				//Sort by name
				struct {
					bool operator()(ResourceBase a, ResourceBase b) const {
						return App->fileSystem->GetLastDirectory(a.assetsPath.c_str()) > App->fileSystem->GetLastDirectory(b.assetsPath.c_str());
					}
				} customTextureSort;

				std::sort(textures.begin(), textures.end(), customTextureSort);

				for (auto it = textures.begin(); it != textures.end(); ++it)
				{
					bool isSelected = true;

					if(emitter->emitterTexture != nullptr)
						isSelected = (strcmp(emitter->emitterTexture->GetAssetsPath(), (*it).assetsPath.c_str()) == 0);

					if (ImGui::Selectable(App->fileSystem->GetLastDirectoryAndFile((*it).assetsPath.c_str()).c_str(), isSelected))
					{
						emitter->SetTexture((*it));
					}
					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			inputTextName = "Add Module - " + emitter->name;
			ImGui::Combo(inputTextName.c_str(), &moduleType, "Add Module\0ParticleMovement\0ParticleColor\0ParticleLifetime\0ParticleRotation\0ParticleSize\0ParticleBillboarding\0EmitterArea");

			inputTextName = "Add Module to " + emitter->name;
			if ((ImGui::Button(inputTextName.c_str())))
			{
				if (moduleType != (int)ParticleModule::Type::NONE)
				{
					emitter->AddModuleFromType((ParticleModule::Type)moduleType);
				}
			}

			ImGui::Separator();

			DisplayParticleModules(emitter);

			inputTextName = "DELETE - " + emitter->name;
			bool toDelete = emitter->toDelete;
			if (ImGui::Checkbox(inputTextName.c_str(), &toDelete))
			{
				emitter->toDelete = toDelete;

			}
		}

		ImGui::Separator(); 
	}
}

void E_Inspector::DisplayParticleModules(Emitter* emitter)
{
	ImGui::TextColored(Cyan.C_Array(), "Particle Modules:");

	std::string inputTextName = emitter->name + " Modules";
	ImGui::BeginChild(inputTextName.c_str(), ImVec2(0.0f, 269.0f), true, ImGuiWindowFlags_HorizontalScrollbar);

	for (auto pModule = emitter->modules.cbegin(); pModule != emitter->modules.cend(); ++pModule)
	{
		if ((*pModule) == nullptr)
			continue;
		
		switch ((*pModule)->type)
		{
		case ParticleModule::Type::EMITTER_BASE:			{ DisplayEmitterBase((*pModule)); }				break;
		case ParticleModule::Type::EMITTER_SPAWN:			{ DisplayEmitterSpawn((*pModule)); }			break;
		case ParticleModule::Type::EMITTER_AREA:			{ DisplayEmitterArea((*pModule)); }				break;
		case ParticleModule::Type::PARTICLE_MOVEMENT:		{ DisplayParticleMovement((*pModule)); }		break;
		case ParticleModule::Type::PARTICLE_COLOR:			{ DisplayParticleColor((*pModule)); }			break;
		case ParticleModule::Type::PARTICLE_LIFETIME:		{ DisplayParticleLifetime((*pModule)); }		break;
		case ParticleModule::Type::PARTICLE_ROTATION:		{ DisplayParticleRotation((*pModule)); }		break;
		case ParticleModule::Type::PARTICLE_SIZE:			{ DisplayParticleSize((*pModule)); }			break;
		case ParticleModule::Type::PARTICLE_BILLBOARDING:	{ DisplayParticleBillboarding((*pModule)); }	break;
		case ParticleModule::Type::NONE:					{  }											break;
		}
	}

	ImGui::EndChild();
}

void E_Inspector::DisplayEmitterBase(ParticleModule* pModule)
{	
	/*bool show = true;
	if (ImGui::CollapsingHeader("Emitter Base", &show, ImGuiTreeNodeFlags_DefaultOpen))*/
	if (ImGui::TreeNodeEx("Emitter Base"/*, ImGuiTreeNodeFlags_DefaultOpen*/))
	{
		EmitterBase* emitterBase = (EmitterBase*)pModule;

		float3 originPos = emitterBase->origin;
		if (ImGui::InputFloat3("OriginPosition", (float*)&originPos, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { emitterBase->origin = originPos; }

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.25f));
		if (ImGui::Button("Delete")) { /*Have access to Owner Emitter through Particle Module*/ }
		ImGui::PopStyleColor();

		ImGui::TreePop();
	}
}

void E_Inspector::DisplayEmitterSpawn(ParticleModule* pModule)
{
	/*bool show = true;
	if (ImGui::CollapsingHeader("Emitter Spawn", &show, ImGuiTreeNodeFlags_DefaultOpen))*/
	if (ImGui::TreeNodeEx("Emitter Spawn"/*, ImGuiTreeNodeFlags_DefaultOpen*/))
	{
		EmitterSpawn* emitterSpawn = (EmitterSpawn*)pModule;

		float ratio = emitterSpawn->spawnRatio;
		if (ImGui::InputFloat("SpawnRatio", &ratio, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { emitterSpawn->spawnRatio = ratio; }

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.25f));
		if (ImGui::Button("Delete")) { /*Have access to Owner Emitter through Particle Module*/ }
		ImGui::PopStyleColor();

		ImGui::TreePop();
	}
}

void E_Inspector::DisplayEmitterArea(ParticleModule* pModule)
{
	if (ImGui::TreeNodeEx("EmitterArea"/*, ImGuiTreeNodeFlags_DefaultOpen*/))
	{
		EmitterArea* emitterArea = (EmitterArea*)pModule;

		float x1 = emitterArea->areaX1;
		float x2 = emitterArea->areaX2;
		float y1 = emitterArea->areaY1;
		float y2 = emitterArea->areaY2;
		float z1 = emitterArea->areaZ1;
		float z2 = emitterArea->areaZ2;

		bool hide = emitterArea->hideArea;
		bool deleteModule = emitterArea->eraseArea;

		if (ImGui::InputFloat("MinX", &x1, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { emitterArea->areaX1 = x1; }
		if (ImGui::InputFloat("MaxX", &x2, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { emitterArea->areaX2 = x2; }
		if (ImGui::InputFloat("MinY", &y1, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { emitterArea->areaY1 = y1; }
		if (ImGui::InputFloat("MaxY", &y2, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { emitterArea->areaY2 = y2; }
		if (ImGui::InputFloat("MinZ", &z1, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { emitterArea->areaZ1 = z1; }
		if (ImGui::InputFloat("MaxZ", &z2, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { emitterArea->areaZ2 = z2; }

		if (ImGui::Checkbox("Hide Area", &hide)) { emitterArea->hideArea = hide; }
		if (ImGui::Checkbox("Delete Area", &deleteModule)) { emitterArea->eraseArea = deleteModule; }

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.25f));
		if (ImGui::Button("Delete")) { /*Have access to Owner Emitter through Particle Module*/ }
		ImGui::PopStyleColor();

		ImGui::TreePop();
	}
}

void E_Inspector::DisplayParticleMovement(ParticleModule* pModule)
{
	/*bool show = true;
	if (ImGui::CollapsingHeader("Particle Movement", &show, ImGuiTreeNodeFlags_DefaultOpen))*/
	if (ImGui::TreeNodeEx("ParticleMovement"/*, ImGuiTreeNodeFlags_DefaultOpen*/))
	{
		ParticleMovement* particleMovement = (ParticleMovement*)pModule;

		float intensity1	= particleMovement->initialIntensity1;
		float intensity2	= particleMovement->initialIntensity2;
		float3 direction1	= particleMovement->initialDirection1;
		float3 direction2	= particleMovement->initialDirection2;
		float3 acceleration1 = particleMovement->initialAcceleration1;
		float3 acceleration2 = particleMovement->initialAcceleration2;
		
		bool hide			= particleMovement->hideMovement;
		bool deleteModule	= particleMovement->eraseMovement;
		
		if (ImGui::InputFloat("InitialIntensity_A", &intensity1, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))	{ particleMovement->initialIntensity1 = intensity1; }
		if (ImGui::InputFloat("InitialIntensity_B", &intensity2, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))	{ particleMovement->initialIntensity2 = intensity2; }
		if (ImGui::InputFloat3("InitialDirection_A", (float*)&direction1, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { particleMovement->initialDirection1 = direction1; }
		if (ImGui::InputFloat3("InitialDirection_B", (float*)&direction2, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { particleMovement->initialDirection2 = direction2; }
		if (ImGui::InputFloat3("InitialAcceleration_A", (float*)&acceleration1, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { particleMovement->initialAcceleration1 = acceleration1; }
		if (ImGui::InputFloat3("InitialAcceleration_B", (float*)&acceleration2, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { particleMovement->initialAcceleration2 = acceleration2; }

		if (ImGui::Checkbox("Hide Movement", &hide))			{ particleMovement->hideMovement = hide; }
		if (ImGui::Checkbox("Delete Movement", &deleteModule))	{ particleMovement->eraseMovement = deleteModule; }

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.25f));
		if (ImGui::Button("Delete")) { /*Have access to Owner Emitter through Particle Module*/ }
		ImGui::PopStyleColor();

		ImGui::TreePop();
	}
}

void E_Inspector::DisplayParticleColor(ParticleModule* pModule)
{
	/*bool show = true;
	if (ImGui::CollapsingHeader("Particle Color", &show, ImGuiTreeNodeFlags_DefaultOpen))*/
	if (ImGui::TreeNodeEx("Particle Color"/*, ImGuiTreeNodeFlags_DefaultOpen*/)) 
	{
		ParticleColor* particleColor = (ParticleColor*)pModule;
		
		Color color1 = particleColor->initialColor1;
		Color color2 = particleColor->initialColor2;
		if (ImGui::InputFloat4("InitialColor1", color1.C_Array(), 4, ImGuiInputTextFlags_EnterReturnsTrue)) { particleColor->initialColor1 = color1; }
		if (ImGui::InputFloat4("InitialColor2", color2.C_Array(), 4, ImGuiInputTextFlags_EnterReturnsTrue)) { particleColor->initialColor2 = color2; }

		bool overLifetime	= particleColor->colorOverLifetime;
		bool hide			= particleColor->hideColor;
		bool deleteModule	= particleColor->eraseColor;
		if (ImGui::Checkbox("Color Over Lifetime", &overLifetime))	{ particleColor->colorOverLifetime = overLifetime; }
		if (ImGui::Checkbox("Hide Color", &hide))			{ particleColor->hideColor = hide; }
		if (ImGui::Checkbox("Delete Color", &deleteModule))	{ particleColor->eraseColor = deleteModule; }

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.25f));
		if (ImGui::Button("Delete")) { /*Have access to Owner Emitter through Particle Module*/ }
		ImGui::PopStyleColor();

		ImGui::TreePop();
	}
}

void E_Inspector::DisplayParticleLifetime(ParticleModule* pModule)
{
	/*bool show = true;
	if (ImGui::CollapsingHeader("Particle Lifetime", &show, ImGuiTreeNodeFlags_DefaultOpen))*/
	if (ImGui::TreeNodeEx("Particle Lifetime"/*, ImGuiTreeNodeFlags_DefaultOpen*/))
	{
		ParticleLifetime* particleLifetime = (ParticleLifetime*)pModule;

		float originLifetime = particleLifetime->initialLifetime;
		if (ImGui::InputFloat("InitialLifetime", &originLifetime, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { particleLifetime->initialLifetime = originLifetime; }
		
		bool hide			= particleLifetime->hideLifetime;
		bool deleteModule	= particleLifetime->eraseLifetime;
		if (ImGui::Checkbox("Hide Lifetime", &hide))			{ particleLifetime->hideLifetime = hide; }
		if (ImGui::Checkbox("Delete Lifetime", &deleteModule))	{ particleLifetime->eraseLifetime = deleteModule; }

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.25f));
		if (ImGui::Button("Delete")) { /*Have access to Owner Emitter through Particle Module*/ }
		ImGui::PopStyleColor();

		ImGui::TreePop();
	}
}

void E_Inspector::DisplayParticleRotation(ParticleModule* pModule)
{

}

void E_Inspector::DisplayParticleSize(ParticleModule* pModule)
{
	if (ImGui::TreeNodeEx("Particle Size"/*, ImGuiTreeNodeFlags_DefaultOpen*/))
	{
		ParticleSize* particleSize = (ParticleSize*)pModule;

		float size1 = particleSize->initialSize1;
		if (ImGui::InputFloat("InitialSize_1", &size1, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { particleSize->initialSize1 = size1; }

		float size2 = particleSize->initialSize2;
		if (ImGui::InputFloat("InitialSize_2", &size2, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue)) { particleSize->initialSize2 = size2; }

		bool changeSizeOverTime = particleSize->SizeOverTime;
		if (ImGui::Checkbox("Size Over Time", &changeSizeOverTime)) { particleSize->SizeOverTime = changeSizeOverTime; }

		bool hide = particleSize->hideSize;
		bool deleteModule = particleSize->eraseSize;
		if (ImGui::Checkbox("Hide Size", &hide)) { particleSize->hideSize = hide; }
		if (ImGui::Checkbox("Delete Size", &deleteModule)) { particleSize->eraseSize = deleteModule; }

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.25f));
		if (ImGui::Button("Delete")) { /*Have access to Owner Emitter through Particle Module*/ }
		ImGui::PopStyleColor();

		ImGui::TreePop();
	}
}

void E_Inspector::DisplayParticleBillboarding(ParticleModule* pModule)
{
	/*bool show = true;
	if (ImGui::CollapsingHeader("Particle Billboarding", &show, ImGuiTreeNodeFlags_DefaultOpen))*/
	if (ImGui::TreeNodeEx("Particle Billboarding", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ParticleBillboarding* particleBillboarding = (ParticleBillboarding*)pModule;

		ImGui::Combo("Billboarding##", &billboardingType, "Screen Aligned\0World Aligned\0X-Axis Aligned\0Y-Axis Aligned\0Z-Axis Aligned");
		ImGui::SameLine();
		if ((ImGui::Button("SELECT"))) { particleBillboarding->billboardingType = (ParticleBillboarding::BillboardingType)billboardingType; }

		bool hideModule		= particleBillboarding->hideBillboarding;
		bool deleteModule	= particleBillboarding->eraseBillboarding;
		if (ImGui::Checkbox("Hide Billboarding", &hideModule))		{ particleBillboarding->hideBillboarding = hideModule; }
		if (ImGui::Checkbox("Delete Billboarding", &deleteModule))	{ particleBillboarding->eraseBillboarding = deleteModule; }

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.25f));
		if (ImGui::Button("Delete")) { /*Have access to Owner Emitter through Particle Module*/ }
		ImGui::PopStyleColor();

		ImGui::TreePop();
	}
}
