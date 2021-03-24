#include <vector>
#include <string>

#include "MathGeoTransform.h"
#include "Profiler.h"
#include "Color.h"
#include "AnimatorClip.h"

#include "Time.h"

#include "EngineApplication.h"
#include "M_Renderer3D.h"
#include "M_Editor.h"
#include "M_Audio.h"
#include "M_FileSystem.h"
#include "M_ResourceManager.h"
#include "M_UISystem.h"
#include "M_Scene.h"
#include "M_ScriptManager.h"

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
#include "C_PlayerController.h"
#include "C_BulletBehavior.h"
#include "C_PropBehavior.h"
#include "C_CameraBehavior.h"
#include "C_GateBehavior.h"
#include "C_Canvas.h"
#include "C_ParticleSystem.h"
#include "C_UI_Image.h"
#include "C_UI_Text.h"

#include "C_Script.h"

#include "R_Shader.h"
#include "R_Texture.h"
#include "R_Script.h"
#include "I_Shaders.h"


#include "E_Inspector.h"

#include <fstream>

#include "MemoryManager.h"

#define MAX_VALUE 100000
#define MIN_VALUE -100000

E_Inspector::E_Inspector() : EditorPanel("Inspector"),
showDeleteComponentPopup	(false),
showTextEditorWindow		(false),
showSaveEditorPopup			(false),
componentType				(0),
mapToDisplay				(0),
componentToDelete			(nullptr),
shaderToRecompile			(nullptr),
texName						("NONE")
{

}

E_Inspector::~E_Inspector()
{
	componentToDelete = nullptr;
}

bool E_Inspector::Draw(ImGuiIO& io)
{
	bool ret = true;


	ImGui::Begin("Inspector");

	SetIsHovered();
	
	GameObject* selected = EngineApp->editor->GetSelectedGameObjectThroughEditor();

	if (selected != nullptr && !selected->is_master_root && !selected->is_scene_root)
	{	
		DrawGameObjectInfo(selected);
		DrawComponents(selected);
		TextEditorWindow();
		ImGui::Separator();

		AddComponentCombo(selected);

		if (showDeleteComponentPopup)
		{
			DeleteComponentPopup(selected);
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
	//bool isStatic = selected_game_object->IsStatic();
	bool isStatic = true;
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
		case ComponentType::PLAYER_CONTROLLER:	{ DrawPlayerControllerComponent((C_PlayerController*)component); }	break;
		case ComponentType::BULLET_BEHAVIOR:	{ DrawBulletBehaviorComponent((C_BulletBehavior*)component); }		break;
		case ComponentType::PROP_BEHAVIOR:		{ DrawPropBehaviorComponent((C_PropBehavior*)component); }			break;
		case ComponentType::CAMERA_BEHAVIOR:	{ DrawCameraBehaviorComponent((C_CameraBehavior*)component); }		break;
		case ComponentType::GATE_BEHAVIOR:		{ DrawGateBehaviorComponent((C_GateBehavior*)component); }			break;
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
		if (cMesh != nullptr)
		{
			// --- IS ACTIVE ---
			bool meshIsActive = cMesh->IsActive();
			if (ImGui::Checkbox("Mesh Is Active", &meshIsActive))
			{
				cMesh->SetIsActive(meshIsActive);
			}

			ImGui::Separator();

			// --- FILE PATH ---
			ImGui::Text("File:");	ImGui::SameLine(); ImGui::TextColored(Green.C_Array(), "%s", cMesh->GetMeshFile());

			ImGui::Separator();

			// --- MESH DATA ---
			ImGui::TextColored(Cyan.C_Array(), "Mesh Data:");

			uint numVertices		= 0;
			uint numNormals		= 0;
			uint numTexCoords		= 0;
			uint numIndices		= 0;
			uint numBones			= 0;

			cMesh->GetMeshData(numVertices, numNormals, numTexCoords, numIndices, numBones);

			ImGui::Text("Vertices:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "  %u", numVertices);
			ImGui::Text("Normals:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "   %u", numNormals);
			ImGui::Text("Tex Coords:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "%u", numTexCoords);
			ImGui::Text("Indices:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "   %u", numIndices);
			ImGui::Text("Bones: ");			ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "    %u",	numBones);

			ImGui::Separator();

			// --- DRAW MODE ---
			ImGui::TextColored(Cyan.C_Array(), "Draw Mode:");

			bool showWireframe		= cMesh->GetShowWireframe();
			bool showBoundingBox	= cMesh->GetShowBoundingBox();
			bool drawVertNormals	= cMesh->GetDrawVertexNormals();
			bool drawFaceNormals	= cMesh->GetDrawFaceNormals();

			if (ImGui::Checkbox("Show Wireframe", &showWireframe))				{ cMesh->SetShowWireframe(showWireframe); }
			if (ImGui::Checkbox("Show Bounding Box", &showBoundingBox))		{ cMesh->SetShowBoundingBox(showBoundingBox); }
			if (ImGui::Checkbox("Draw Vertex Normals", &drawVertNormals))		{ cMesh->SetDrawVertexNormals(drawVertNormals); }
			if (ImGui::Checkbox("Draw Face Normals", &drawFaceNormals))		{ cMesh->SetDrawFaceNormals(drawFaceNormals); }
		}
		else
		{
			LOG("[ERROR] Could not get the Mesh Component from %s Game Object!", cMesh->GetOwner()->GetName());
		}

		if (!show)
		{
			componentToDelete				= cMesh;
			showDeleteComponentPopup		= true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawMaterialComponent(C_Material* cMaterial)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Material", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cMaterial != nullptr)
		{
			bool materialIsActive = cMaterial->IsActive();
			if (ImGui::Checkbox("Material Is Active", &materialIsActive))
			{
				cMaterial->SetIsActive(materialIsActive);
			}

			ImGui::Separator();

			// --- MATERIAL PATH ---
			ImGui::Text("File:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", cMaterial->GetTextureFile());

			ImGui::Separator();

			// --- MATERIAL COLOR & ALPHA ---
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Material Data:");

			Color color = cMaterial->GetMaterialColour();

			if (ImGui::ColorEdit3("Diffuse Color", (float*)&color, ImGuiColorEditFlags_NoAlpha))
			{
				cMaterial->SetMaterialColour(color);
			}

			if (ImGui::SliderFloat("Diffuse Alpha", (float*)&color.a, 0.0f, 1.0f, "%.3f"))
			{
				cMaterial->SetMaterialColour(color);
			}

			if (ImGui::Button("Save Material"))
			{
				//App->resourceManager->SaveResourceToLibrary((Resource*)cMaterial->GetMaterial());
			}

			ImGui::Separator();

			// --- SHADER DATA ---
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Shader Data:");

			ImGui::Text("Shader Active:");
			ImGui::SameLine();

			if (cMaterial->GetShader() != nullptr)
			{
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), cMaterial->GetShader()->GetAssetsFile());

				if (allShaders.empty()) App->resourceManager->GetAllShaders(allShaders);
				shaderName = cMaterial->GetShader()->GetAssetsFile();
				if (ImGui::BeginCombo("Shader", cMaterial->GetShader()->GetAssetsFile(), ImGuiComboFlags_PopupAlignLeft))
				{
					for (uint i = 0; i < allShaders.size(); i++)
					{
						const bool selectedShader = (shaderName == allShaders[i]->GetAssetsFile());
						if (ImGui::Selectable(allShaders[i]->GetAssetsFile(), selectedShader))
						{
							cMaterial->SetShader(allShaders[i]);

							shaderName = allShaders[i]->GetAssetsFile();
						}
					}
					ImGui::EndCombo();
				}

				if (ImGui::Button("Edit Shader"))
				{
					CallTextEditor(cMaterial);
				}

				R_Shader* shader = cMaterial->GetShader();
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

				if (!cMaterial->GetShader()->uniforms.empty())
				{
					if (ImGui::Button("Save Uniforms"))
					{
						App->resourceManager->SaveResourceToLibrary(cMaterial->GetShader());
					}
				}
			}
			ImGui::Separator();

			// --- TEXTURE DATA ---

			/*if (allTextures.empty()) 
				App->resourceManager->GetAllTextures(allTextures);
			
			if (texName == "NONE" && cMaterial->GetTexture())
			{
				texName = cMaterial->GetTexture()->GetAssetsFile();
			}
			if (ImGui::BeginCombo("Texture", texName.c_str(), ImGuiComboFlags_PopupAlignLeft))
			{
				for (uint i = 0; i < allTextures.size(); i++)
				{
					const bool selectedShader = (texName.c_str() == allTextures[i]->GetAssetsFile());
					if (ImGui::Selectable(allTextures[i]->GetAssetsFile(), selectedShader))
					{
						cMaterial->SwapTexture(allTextures[i]);

						texName = allTextures[i]->GetAssetsFile();
					}
				}
				ImGui::EndCombo();
			}*/

			DisplayTextureData(cMaterial);

			ImGui::Separator();

			// --- MAIN MAPS ---
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Main Maps:");

			if (ImGui::Combo("Textures(WIP)", &mapToDisplay, "Diffuse\0Specular\0Ambient\0Height\0Normal\0"))
			{
				LOG("[SCENE] Changed to map %d", mapToDisplay);
			}

			bool useCheckeredTex = cMaterial->UseDefaultTexture();
			if (ImGui::Checkbox("Use Default Texture", &useCheckeredTex))
			{
				cMaterial->SetUseDefaultTexture(useCheckeredTex);
			}

			// --- TEXTURE DISPLAY ---
			TextureDisplay(cMaterial);
		}
		else
		{
			LOG("[ERROR] Could not get the Material Component from %s Game Object!", cMaterial->GetOwner()->GetName());
		}

		if (!show)
		{
			componentToDelete				= cMaterial;
			showDeleteComponentPopup		= true;
		}

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
	bool show = true;
	if (ImGui::CollapsingHeader("Animator", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cAnimator != nullptr)
		{
			bool animationIsActive	= cAnimator->IsActive();
			if (ImGui::Checkbox("Is Active", &animationIsActive))	{ cAnimator->SetIsActive(animationIsActive); }
			
			ImGui::SameLine(ImGui::GetWindowWidth() * 0.69f);

			std::string animatorStateString = cAnimator->GetAnimatorStateAsString();
			ImGui::Text("State:"); ImGui::SameLine(); ImGui::TextColored(&Yellow, "{ %s }", animatorStateString.c_str());

			ImGui::Separator();

			// --- ANIMATOR VARIABLES
			static int selectedClip				= 0;
			std::string clipNamesString			= cAnimator->GetClipNamesAsString();

			float speed							= cAnimator->GetPlaybackSpeed();
			float minSpeed						= 0.1f;
			float maxSpeed						= 10.0f;
			
			bool interpolate					= cAnimator->GetInterpolate();
			bool loopAnimation					= cAnimator->GetLoopAnimation();
			bool playOnStart					= cAnimator->GetPlayOnStart();
			bool cameraCulling					= cAnimator->GetCameraCulling();
			bool showBones						= cAnimator->GetShowBones();

			// -- CURRENT CLIP VARIABLES
			AnimatorClip* currentClip			= cAnimator->GetCurrentClip();

			if (currentClip == nullptr)
			{
				AnimatorClip Clip = AnimatorClip();
				currentClip = &Clip;
			}

			const char* animationName			= currentClip->GetAnimationName();
			float animationTicksPerSecond		= currentClip->GetAnimationTicksPerSecond();
			float animationDuration				= currentClip->GetAnimationDuration();

			const char* currentClipName			= currentClip->GetName();
			uint currentClipStart				= currentClip->GetStart();
			uint currentClipEnd					= currentClip->GetEnd();
			float currentClipDuration			= currentClip->GetDuration();
			bool currentClipLoop				= currentClip->IsLooped();

			float clipCime						= currentClip->GetClipTime();
			float clipFrame						= currentClip->GetClipFrame();
			uint clipTicks						= currentClip->GetClipTick();

			// --- NEW CLIP VARIABLES
			static int selectedAnimation		= 0;
			std::string animationNames			= cAnimator->GetAnimationNamesAsString();

			static char newClipName[128]		= "Enter Clip Name";
			ImGuiInputTextFlags inputTxtFlags	= ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;

			static int newClipStart				= 0;
			static int newClipEnd				= (int)animationDuration;
			int newClipMin						= 0;
			int newClipMax						= (int)animationDuration;
			static bool loop					= false;

			static bool success					= false;																			// --- TODO: Transform into non-static variables later.
			static bool textTimerRunning		= false;																			//
			static float textTimer				= 0.0f;																				//
			static float textDuration			= 2.5f;																				// ----------------------------------------------------
			
			// --- EXISTING CLIPS VARIABLES
			std::vector<std::string> clipNames = cAnimator->GetClipNamesAsVector();

			// --- DISPLAY
			if (ImGui::BeginTabBar("AnimatorTabBar", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("Settings & Controls"))
				{
					// --- ANIMATOR SETTINGS
					ImGui::TextColored(Cyan.C_Array(), "Animation Settings");

					if (ImGui::Combo("Select Clip", &selectedClip, clipNamesString.c_str()))
					{
						char selected_name = clipNamesString[selectedClip];
						cAnimator->SetCurrentClipByIndex((uint)selectedClip);
					}

					if (ImGui::Button("Play"))	{ cAnimator->Play(); }	ImGui::SameLine();
					if (ImGui::Button("Pause"))	{ cAnimator->Pause(); }	ImGui::SameLine();
					if (ImGui::Button("Step"))	{ cAnimator->Step(); }	ImGui::SameLine();
					if (ImGui::Button("Stop"))	{ cAnimator->Stop(); }

					if (ImGui::SliderFloat("Playback Speed", &speed, minSpeed, maxSpeed, "X %.3f", 0)) { cAnimator->SetPlaybackSpeed(speed); }

					if (ImGui::Checkbox("Interpolate", &interpolate))		{ cAnimator->SetInterpolate(interpolate); }
					if (ImGui::Checkbox("Loop Animation", &loopAnimation))	{ cAnimator->SetLoopAnimation(loopAnimation); }
					if (ImGui::Checkbox("Play On Start", &playOnStart))		{ cAnimator->SetPlayOnStart(playOnStart); }
					if (ImGui::Checkbox("Camera Culling", &cameraCulling))	{ cAnimator->SetCameraCulling(cameraCulling); }
					if (ImGui::Checkbox("Show Bones", &showBones))			{ cAnimator->SetShowBones(showBones); }

					ImGui::Separator();

					// --- ANIMATOR STATS
					ImGui::TextColored(Cyan.C_Array(), "Animation Stats");

					ImGui::Text("Name:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "             %s",		animationName);
					ImGui::Text("Ticks Per Second:");	ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), " %.3f",				animationTicksPerSecond);
					ImGui::Text("Duration:");			ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "         %.3f",		animationDuration);

					ImGui::Separator();

					ImGui::TextColored(Cyan.C_Array(), "Clip Stats");

					ImGui::Text("Name:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "             %s",		currentClipName);
					ImGui::Text("Time:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "             %.3f",	clipCime);
					ImGui::Text("Frame:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "            %.3f",	clipFrame);
					ImGui::Text("Tick:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "		     %u",		clipTicks);
					ImGui::Text("Range:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "            %u - %u", currentClipStart, currentClipEnd);
					ImGui::Text("Duration:");			ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "         %.3f",		currentClipDuration);
					ImGui::Text("Loop:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "             %s",		(currentClipLoop) ? "True" : "False");

					ImGui::Separator();

					// --- ANIMATOR DEBUG CONTROLS
					ImGui::TextColored(Cyan.C_Array(), "Debug Controls");

					if (ImGui::Button("Previous Keyframe"))		{ cAnimator->StepToPrevKeyframe(); }	ImGui::SameLine(150.0f);
					if (ImGui::Button("Next Keyframe"))			{ cAnimator->StepToNextKeyframe(); }
					if (ImGui::Button("Refresh Bone Display"))	{ cAnimator->RefreshBoneDisplay(); }

					ImGui::EndTabItem();
				}
				
				if (ImGui::BeginTabItem("Clip Manager"))
				{
					ImGui::Separator();
					ImGui::Separator();
					
					ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
					ImGui::BeginChild("Clip Manager Child", ImVec2(0.0f, 170.0f), true);

					ImGui::TextColored(Cyan.C_Array(), "Create Clip");

					ImGui::Combo("Select Animation", &selectedAnimation, animationNames.c_str());
					ImGui::InputText("Clip Name", newClipName, IM_ARRAYSIZE(newClipName), inputTxtFlags);
					ImGui::SliderInt("Clip Start", &newClipStart, newClipMin, newClipMax);
					ImGui::SliderInt("Clip End", &newClipEnd, newClipMin, newClipMax);
					ImGui::Checkbox("Loop Clip", &loop);

					if (newClipStart > newClipEnd) { newClipEnd = newClipStart; };

					if (ImGui::Button("Create")) 
					{ 
						if (EngineApp->gameState != GameState::PLAY)
						{
							success = cAnimator->AddClip(AnimatorClip(cAnimator->GetAnimationByIndex((uint)selectedAnimation), newClipName, newClipStart, newClipEnd, loop));
							textTimerRunning = true;
						}
						else
						{
							ImGui::SameLine();
							ImGui::TextColored(Red.C_Array(), "Cannot Create Clips While in Game Mode!");
						}
					}

					if (textTimerRunning)
					{	
						ImGui::SameLine();
						
						if (success)
						{
							static std::string new_clip_name_str = newClipName;
							ImGui::TextColored(Green.C_Array(), "Successfully Created Clip { %s }", new_clip_name_str.c_str());

							strcpy_s(newClipName, 128, "Enter Clip Name");																// --- Re-setting the New Clip Parameters
							newClipStart	= 0;																						// 
							newClipEnd		= (int)animationDuration;																	// 
							loop			= false;																					// --------------------------------------
						}
						else
						{
							ImGui::TextColored(Red.C_Array(), "A clip with the same name already exists!");
						}

						textTimer += Time::Real::GetDT();
						if (textTimer > textDuration)
						{
							textTimerRunning = false;
							textTimer = 0.0f;
						}
					}

					ImGui::EndChild();
					ImGui::PopStyleVar();

					ImGui::Separator();
					ImGui::Separator();

					ImGui::TextColored(Cyan.C_Array(), "Existing Clips");

					for (uint i = 0; i < clipNames.size(); ++i)
					{	
						if (ImGui::TreeNodeEx(clipNames[i].c_str(), ImGuiTreeNodeFlags_Bullet))
						{
							ImGui::TreePop();
						}

						/*if (ImGui::IsItemHovered())
						{
							ImGui::OpenPopup("ClipPopUp");
							if (ImGui::BeginPopup("Clip Info"))
							{
								ImGui::EndPopup();
							}
						}*/
					}

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
	if (ImGui::CollapsingHeader("Audio Source", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (cAudioSource != nullptr)
		{
			unsigned int currentEvent = cAudioSource->GetEventId();
			const std::string& currentEventName = cAudioSource->GetEventName();
			//cAudioSource->GetEvent(&currentEventName,&currentEvent);

			ImGui::Text("Event playing %s", currentEventName.c_str());
			ImGui::Text("Event id %u", currentEvent);

			if (ImGui::BeginCombo("##Audio", currentEventName.c_str()))
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
			if (ImGui::DragFloat("Volume", &volume, 0.01f, 0.01f, 1.0f))
			{
				cAudioSource->SetVolume(volume);
			}

			if ((ImGui::Button("Play")))
			{
				cAudioSource->PlayFx(currentEvent);
			}
			if ((ImGui::Button("Stop")))
			{
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
			bool isActive = cCanvas->IsActive();
			if (ImGui::Checkbox("Canvas Is Active", &isActive)) { cCanvas->SetIsActive(isActive); }

			bool isInvisible = cCanvas->IsInvisible();
			if (ImGui::Checkbox("Canvas Is Invisible", &isInvisible)) { cCanvas->SetIsInvisible(isInvisible); }

			ImGui::Separator();

			ImGui::TextColored(Cyan.C_Array(), "Canvas Settings:");

			ImGui::Separator();

			// --- RECT ---
			float2 size = { cCanvas->GetRect().w, cCanvas->GetRect().h };
			float2 pivot = { cCanvas->pivot.x, cCanvas->pivot.y };

			if (ImGui::DragFloat2("Rect", (float*)&size, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
			{
				if (size.x < 0)
					size.x = 0;
				if (size.y < 0)
					size.y = 0;
					
				
					
				cCanvas->SetSize(size);

				if (pivot.x < cCanvas->GetPosition().x - cCanvas->GetSize().x / 2)
					pivot.x = cCanvas->GetPosition().x - cCanvas->GetSize().x / 2;

				if (pivot.x > cCanvas->GetPosition().x + cCanvas->GetSize().x / 2)
					pivot.x = cCanvas->GetPosition().x + cCanvas->GetSize().x / 2;


				if (pivot.y < cCanvas->GetPosition().y - cCanvas->GetSize().y / 2)
					pivot.y = cCanvas->GetPosition().y - cCanvas->GetSize().y / 2;

				if (pivot.y > cCanvas->GetPosition().y + cCanvas->GetSize().y / 2)
					pivot.y = cCanvas->GetPosition().y + cCanvas->GetSize().y / 2;

				cCanvas->pivot.x = pivot.x;
				cCanvas->pivot.y = pivot.y;
			}

			// --- PIVOT ---
			if (ImGui::DragFloat2("Pivot", (float*)&pivot, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
			{

				if (pivot.x < cCanvas->GetPosition().x - cCanvas->GetSize().x / 2)
					pivot.x = cCanvas->GetPosition().x - cCanvas->GetSize().x / 2;

				if (pivot.x > cCanvas->GetPosition().x + cCanvas->GetSize().x / 2)
					pivot.x = cCanvas->GetPosition().x + cCanvas->GetSize().x / 2;


				if (pivot.y < cCanvas->GetPosition().y - cCanvas->GetSize().y / 2)
					pivot.y = cCanvas->GetPosition().y - cCanvas->GetSize().y / 2;

				if (pivot.y > cCanvas->GetPosition().y + cCanvas->GetSize().y / 2)
					pivot.y = cCanvas->GetPosition().y + cCanvas->GetSize().y / 2;

				cCanvas->pivot.x = pivot.x;
				cCanvas->pivot.y = pivot.y;
			}

			if (ImGui::Button("Reset Pivot"))
				cCanvas->pivot = cCanvas->GetPosition();

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
	bool show = true;
	if (ImGui::CollapsingHeader("Particle System", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{

		if (!show)
		{
			componentToDelete = cParticleSystem;
			showDeleteComponentPopup = true;
		}

		ImGui::Separator();
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

		C_Canvas* canvas = image->GetOwner()->parent->GetComponent<C_Canvas>();

		if (ImGui::DragFloat2("Image Size", (float*)&size, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
		{
			if (size.x < 0)
				size.x = 0;
			if (size.y < 0)
				size.y = 0;

			image->SetW(size.x);
			image->SetH(size.y);
		}

		if (ImGui::DragFloat2("Image Pos", (float*)&pos, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
		{
			if (pos.x - size.x / 2 < canvas->GetPosition().x - canvas->GetSize().x / 2)
				pos.x = canvas->GetPosition().x - canvas->GetSize().x / 2 + size.x / 2;

			if (pos.x + size.x / 2 > canvas->GetPosition().x + canvas->GetSize().x / 2)
				pos.x = canvas->GetPosition().x + canvas->GetSize().x / 2 - size.x / 2;


			if (pos.y - size.y / 2 < canvas->GetPosition().y - canvas->GetSize().y / 2)
				pos.y = canvas->GetPosition().y - canvas->GetSize().y / 2 + size.y / 2;

			if (pos.y + size.y / 2 > canvas->GetPosition().y + canvas->GetSize().y / 2)
				pos.y = canvas->GetPosition().y + canvas->GetSize().y / 2 - size.y / 2;

			image->SetX(pos.x);
			image->SetY(pos.y);
		}
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

		ImGui::Text(text->text.c_str());
	}
	ImGui::Separator();

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
			const std::map<std::string, std::string> scripts = EngineApp->scriptManager->GetAviableScripts();
			
			std::string select;
			if(scripts.size() != 0)
				select = (*scripts.begin()).first;

			if (ImGui::BeginCombo("##Select Script", "SelectScript", ImGuiComboFlags_PopupAlignLeft))
			{
				for (std::map<std::string, std::string>::const_iterator it = scripts.cbegin(); it != scripts.cend(); ++it)
				{
					bool selectedScript = (select == (*it).first.c_str());
					if (ImGui::Selectable((*it).first.c_str(), selectedScript)) 
					{
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
		if (!show)
			cScript->GetOwner()->DeleteComponent(cScript);

		ImGui::Separator();
		return;
	}

	if (ImGui::CollapsingHeader(cScript->GetDataName().c_str(), &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		//TODO: Show inspector variables
		ImGui::Text("Name %s", cScript->GetDataName().c_str());
	}
	if (!show)
	{
		componentToDelete = cScript;
		showDeleteComponentPopup = true;
	}
	ImGui::Separator();

}

void E_Inspector::DrawPlayerControllerComponent(C_PlayerController* cController)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Player Controller", &show, ImGuiTreeNodeFlags_Leaf))
	{
		bool isActive = cController->IsActive();
		if (ImGui::Checkbox("Controller Is Active", &isActive))
			cController->SetIsActive(isActive);

		ImGui::Separator();
		if (ImGui::TreeNodeEx("Character"))
		{
			float speed = cController->Speed();
			if (ImGui::InputFloat("Speed", &speed, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
				cController->SetSpeed(speed);

			ImGui::Separator();

			uint state = (uint)cController->state;
			ImGui::Text("Player State:"); ImGui::SameLine(); ImGui::TextColored(Yellow.C_Array(), "{ %u }", state);

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Weapon"))
		{
			float bulletSpeed = cController->BulletSpeed();
			if (ImGui::InputFloat("Bullet Speed", &bulletSpeed, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
				cController->SetBulletSpeed(bulletSpeed);

			float fireRate = cController->FireRate();
			if (ImGui::InputFloat("Fire Rate", &fireRate, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
				cController->SetFireRate(fireRate);

			int ammo = cController->CurrentAmmo();
			if (ImGui::InputInt("Ammo", &ammo, 1, 10, ImGuiInputTextFlags_EnterReturnsTrue))
				cController->SetCurrentAmmo(ammo);

			int maxAmmo = cController->MaxAmmo();
			if (ImGui::InputInt("Max Ammo", &maxAmmo, 1, 10, ImGuiInputTextFlags_EnterReturnsTrue))
				cController->SetMaxAmmo(maxAmmo);

			bool automatic = cController->IsAutomatic();
			if (ImGui::Checkbox("Automatic", &automatic))
				cController->SetAutomatic(automatic);

			ImGui::Separator();

			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("Dash"))
		{
			float dashSpeed = cController->DashSpeed();
			if (ImGui::InputFloat("Dash Speed", &dashSpeed, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
				cController->SetDashSpeed(dashSpeed);

			float dashTime = cController->DashTime();
			if (ImGui::InputFloat("Dash Time", &dashTime, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
				cController->SetDashTime(dashTime);

			float dashColdown = cController->DashColdown();
			if (ImGui::InputFloat("Dash Coldown", &dashColdown, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
				cController->SetDashColdown(dashColdown);

			ImGui::TreePop();
		}

		if (!show)
		{
			componentToDelete = cController;
			showDeleteComponentPopup = true;
		}

		ImGui::Separator();
	}
	return;
}

void E_Inspector::DrawBulletBehaviorComponent(C_BulletBehavior* cBehavior)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Bullet Bahavior", &show, ImGuiTreeNodeFlags_Leaf))
	{
		bool isActive = cBehavior->IsActive();
		if (ImGui::Checkbox("Bullet Is Active", &isActive))
			cBehavior->SetIsActive(isActive);

		ImGui::Separator();

		float autodestruct = cBehavior->GetAutodestruct();
		if (ImGui::InputFloat("Autodestruction", &autodestruct, 1, 1, 4, ImGuiInputTextFlags_EnterReturnsTrue))
			cBehavior->SetAutodestruct(autodestruct);

		if (!show)
		{
			componentToDelete = cBehavior;
			showDeleteComponentPopup = true;
		}

		ImGui::Separator();
	}
	return;
}

void E_Inspector::DrawPropBehaviorComponent(C_PropBehavior* cBehavior)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Prop Bahavior", &show, ImGuiTreeNodeFlags_Leaf))
	{
		bool isActive = cBehavior->IsActive();
		if (ImGui::Checkbox("Prop Is Active", &isActive))
			cBehavior->SetIsActive(isActive);

		if (!show)
		{
			componentToDelete = cBehavior;
			showDeleteComponentPopup = true;
		}

		ImGui::Separator();
	}
	return;
}

void E_Inspector::DrawCameraBehaviorComponent(C_CameraBehavior* cBehavior)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Camera Bahavior", &show, ImGuiTreeNodeFlags_Leaf))
	{
		bool isActive = cBehavior->IsActive();
		if (ImGui::Checkbox("Camera Behavior Is Active", &isActive))
			cBehavior->SetIsActive(isActive);

		ImGui::Separator();

		float3 offset = cBehavior->GetOffset();
		float o[3] = { offset.x, offset.y, offset.z };
		if (ImGui::InputFloat3("Offset", o, 4, ImGuiInputTextFlags_EnterReturnsTrue))
			cBehavior->SetOffset(float3(o[0], o[1], o[2]));

		if (!show)
		{
			componentToDelete = cBehavior;
			showDeleteComponentPopup = true;
		}

		ImGui::Separator();
	}
	return;
}

void E_Inspector::DrawGateBehaviorComponent(C_GateBehavior* cBehavior)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Gate Bahavior", &show, ImGuiTreeNodeFlags_Leaf))
	{
		bool isActive = cBehavior->IsActive();
		if (ImGui::Checkbox("Gate Is Active", &isActive))
			cBehavior->SetIsActive(isActive);

		ImGui::Separator();

		if (!show)
		{
			componentToDelete = cBehavior;
			showDeleteComponentPopup = true;
		}

		ImGui::Separator();
	}
	return;
}

void E_Inspector::AddComponentCombo(GameObject* selectedGameObject)
{
	ImGui::Combo("##", &componentType, "Add Component\0Transform\0Mesh\0Material\0Light\0Camera\0Animator\0Animation\0RigidBody\0Box Collider\0Sphere Collider\0Capsule Collider\0Particle System\0Canvas\0Audio Source\0Audio Listener\0Script\0Player Controller\0Bullet Behavior\0Prop Behavior\0Camera Behavior\0Gate Behavior\0UI Image\0UI Text");

	ImGui::SameLine();

	if ((ImGui::Button("ADD")))
	{ 
		if (componentType != (int)ComponentType::NONE)
		{
			if (componentType == (int)ComponentType::UI_IMAGE)
				AddUIComponent(selectedGameObject, ComponentType::UI_IMAGE);
			else if(componentType == (int)ComponentType::UI_TEXT)
				AddUIComponent(selectedGameObject, ComponentType::UI_TEXT);

			else
				selectedGameObject->CreateComponent((ComponentType)componentType);
		}
	}
}

void E_Inspector::AddUIComponent(GameObject* selectedGameObject, ComponentType type)
{
	if (type == ComponentType::UI_IMAGE)
	{
		// Option 1: selectedGameObject has a canvas
		if (selectedGameObject->GetComponent<C_Canvas>() != nullptr)
		{
			GameObject* newGO;
			newGO = App->scene->CreateGameObject("UI Image", selectedGameObject);
			newGO->CreateComponent(ComponentType::UI_IMAGE);
		}
		// Option 2: selectedGameObject's parent has a canvas
		else if (selectedGameObject->parent->GetComponent<C_Canvas>() != nullptr)
		{
			selectedGameObject->SetName("UI Image");
			selectedGameObject->CreateComponent(ComponentType::UI_IMAGE);
		}
		// Option 3: need to crete a canvas
		else
		{
			selectedGameObject->SetName("Canvas");
			selectedGameObject->CreateComponent(ComponentType::CANVAS);

			GameObject* newImage = App->scene->CreateGameObject("UI Image", selectedGameObject);
			newImage->CreateComponent(ComponentType::UI_IMAGE);
		}
	}

	else if (type == ComponentType::UI_TEXT)
	{
		// Option 1: selectedGameObject has a canvas
		if (selectedGameObject->GetComponent<C_Canvas>() != nullptr)
		{
			GameObject* newGO;
			newGO = App->scene->CreateGameObject("UI Text", selectedGameObject);
			newGO->CreateComponent(ComponentType::UI_TEXT);
		}
		// Option 2: selectedGameObject's parent has a canvas
		else if (selectedGameObject->parent->GetComponent<C_Canvas>() != nullptr)
		{
			selectedGameObject->SetName("UI Text");
			selectedGameObject->CreateComponent(ComponentType::UI_TEXT);
		}
		// Option 3: need to crete a canvas
		else
		{
			selectedGameObject->SetName("Canvas");
			selectedGameObject->CreateComponent(ComponentType::CANVAS);

			GameObject* newText = App->scene->CreateGameObject("UI Text", selectedGameObject);
			newText->CreateComponent(ComponentType::UI_TEXT);
		}
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

void E_Inspector::DisplayTextureData(C_Material* cMaterial)
{
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Texture Data:");

	uint id					= 0;
	uint width				= 0;
	uint height				= 0;
	uint depth				= 0;
	uint bpp				= 0;
	uint size				= 0;
	std::string format		= "NONE";
	bool compressed			= 0;

	cMaterial->GetTextureInfo(id, width, height, depth, bpp, size, format, compressed);

	ImGui::Text("Texture ID:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%u", id);
	ImGui::Text("Width:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "     %upx", width);
	ImGui::Text("Height:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %upx", height);
	ImGui::Text("Depth:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "     %u", depth);
	ImGui::Text("Bpp:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "       %uB", bpp);
	ImGui::Text("Size:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "      %uB", size);
	ImGui::Text("Format:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %s", format.c_str());
	ImGui::Text("Compressed:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", compressed ? "True" : "False");
}

void E_Inspector::TextureDisplay(C_Material* cMaterial)
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
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Texture Display:");

		ImGui::Spacing();

		ImGui::Image(texId, displaySize, ImVec2(1.0f, 0.0f), ImVec2(0.0f, 1.0f), tint, borderColor);			// ImGui has access to OpenGL's buffers, so only the Texture Id is required.
	}
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

				showSaveEditorPopup = false;
				showTextEditorWindow = true;

				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Don't Save"))
			{
				showSaveEditorPopup = false;
				showTextEditorWindow = true;

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