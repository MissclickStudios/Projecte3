#include <vector>
#include <string>

#include "MathGeoTransform.h"

#include "Color.h"
#include "AnimatorClip.h"

#include "Time.h"

#include "Application.h"
#include "M_Renderer3D.h"
#include "M_Editor.h"

#include "GameObject.h"
#include "Component.h"
#include "C_Transform.h"
#include "C_Mesh.h"
#include "C_Material.h"
#include "C_Light.h"
#include "C_Camera.h"
#include "C_Animator.h"
#include "C_Animation.h"

#include "E_Inspector.h"

#define MAX_VALUE 100000
#define MIN_VALUE -100000

E_Inspector::E_Inspector() : EditorPanel("Inspector"),
showDeleteComponentPopup	(false),
componentType				(0),
mapToDisplay				(0),
componentToDelete			(nullptr)
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
	
	GameObject* selected = App->editor->GetSelectedGameObjectThroughEditor();

	if (selected != nullptr && !selected->is_master_root && !selected->is_scene_root)
	{	
		DrawGameObjectInfo(selected);
		DrawComponents(selected);

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
		case ComponentType::TRANSFORM:	{ DrawTransformComponent((C_Transform*)component); }	break;
		case ComponentType::MESH:		{ DrawMeshComponent((C_Mesh*)component); }				break;
		case ComponentType::MATERIAL:	{ DrawMaterialComponent((C_Material*)component); }		break;
		case ComponentType::LIGHT:		{ DrawLightComponent((C_Light*)component); }			break;
		case ComponentType::CAMERA:	{ DrawCameraComponent((C_Camera*)component); }			break;
		case ComponentType::ANIMATOR:	{ DrawAnimatorComponent((C_Animator*)component); }		break;
		case ComponentType::ANIMATION: { DrawAnimationComponent((C_Animation*)component); }	break;
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

			ImGui::Separator();

			// --- TEXTURE DATA ---
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
			
			ImGui::Text("WORK IN PROGRESS");
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
				App->editor->SetCurrentCameraThroughEditor(cCamera);
			}

			if (ImGui::Button("Return to Master Camera"))
			{
				App->editor->SetMasterCameraThroughEditor();
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
			if (ImGui::Checkbox("Is Active", &animationIsActive))							{ cAnimator->SetIsActive(animationIsActive); }

			ImGui::Separator();

			// --- ANIMATOR VARIABLES
			static int selectedClip			= 0;
			std::string clipNamesString		= cAnimator->GetClipNamesAsString();

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
				currentClip = &AnimatorClip();
			}

			const char* animationName			= currentClip->GetAnimationName();
			float animationTicksPerSecond	= currentClip->GetAnimationTicksPerSecond();
			float animationDuration			= currentClip->GetAnimationDuration();

			const char* currentClipName		= currentClip->GetName();
			uint currentClipStart				= currentClip->GetStart();
			uint currentClipEnd				= currentClip->GetEnd();
			float currentClipDuration			= currentClip->GetDuration();
			bool currentClipLoop				= currentClip->IsLooped();

			float clipCime						= currentClip->GetClipTime();
			float clipFrame					= currentClip->GetClipFrame();
			uint clipTicks						= currentClip->GetClipTick();

			// --- NEW CLIP VARIABLES
			static int selectedAnimation		= 0;
			std::string animationNames			= cAnimator->GetAnimationNamesAsString();

			static char newClipName[128]		= "Enter Clip Name";
			ImGuiInputTextFlags inputTxtFlags	= ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;

			static int newClipStart			= 0;
			static int newClipEnd				= (int)animationDuration;
			int newClipMin					= 0;
			int newClipMax					= (int)animationDuration;
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

					if (ImGui::Button("Play"))									{ cAnimator->Play(); }		ImGui::SameLine();
					if (ImGui::Button("Pause"))									{ cAnimator->Pause(); }	ImGui::SameLine();
					if (ImGui::Button("Step"))									{ cAnimator->Step(); }		ImGui::SameLine();
					if (ImGui::Button("Stop"))									{ cAnimator->Stop(); }

					if (ImGui::SliderFloat("Playback Speed", &speed, minSpeed, maxSpeed, "X %.3f", 0)) { cAnimator->SetPlaybackSpeed(speed); }

					if (ImGui::Checkbox("Interpolate", &interpolate))			{ cAnimator->SetInterpolate(interpolate); }
					if (ImGui::Checkbox("Loop Animation", &loopAnimation))		{ cAnimator->SetLoopAnimation(loopAnimation); }
					if (ImGui::Checkbox("Play On Start", &playOnStart))			{ cAnimator->SetPlayOnStart(playOnStart); }
					if (ImGui::Checkbox("Camera Culling", &cameraCulling))		{ cAnimator->SetCameraCulling(cameraCulling); }
					if (ImGui::Checkbox("Show Bones", &showBones))				{ cAnimator->SetShowBones(showBones); }

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
						if (!App->play)
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
							newClipStart	= 0;																							// 
							newClipEnd	= (int)animationDuration;																		// 
							loop			= false;																						// --------------------------------------
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
	static bool show = true;
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

void E_Inspector::AddComponentCombo(GameObject* selectedGameObject)
{
	ImGui::Combo("##", &componentType, "Add Component\0Transform\0Mesh\0Material\0Light\0Camera\0Animator\0Animation");

	ImGui::SameLine();

	if ((ImGui::Button("ADD")))
	{ 
		if (componentType != (int)ComponentType::NONE)
		{
			selectedGameObject->CreateComponent((ComponentType)componentType);
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
		texId = (ImTextureID)App->renderer->GetDebugTextureID();
		//tex_id = (ImTextureID)App->renderer->GetSceneRenderTexture();
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