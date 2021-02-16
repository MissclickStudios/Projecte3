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
void E_Inspector::DrawGameObjectInfo(GameObject* selected_game_object)
{
	// --- IS ACTIVE ---
	bool gameObjectIsActive = selected_game_object->IsActive();
	if (ImGui::Checkbox("Is Active", &gameObjectIsActive))
	{
		selected_game_object->SetIsActive(gameObjectIsActive);
	}

	ImGui::SameLine();

	// --- GAME OBJECT'S NAME ---
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.33f);
	static char buffer[64];
	strcpy_s(buffer, selected_game_object->GetName());
	if (ImGui::InputText("Name", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
	{
		selected_game_object->SetName(buffer);
	}

	ImGui::SameLine(); HelpMarker("Press ENTER to Rename");

	ImGui::SameLine();

	// --- IS STATIC ---
	//bool is_static = selected_game_object->IsStatic();
	bool isStatic = true;
	if (ImGui::Checkbox("Is Static", &isStatic))
	{
		selected_game_object->SetIsStatic(isStatic);
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

void E_Inspector::DrawComponents(GameObject* selected_game_object)
{
	if (selected_game_object == nullptr)
	{
		LOG("[ERROR] Editor Inspector: Could not draw the selected GameObject's components! Error: Selected GameObject was nullptr.");
		return;
	}
	
	for (uint i = 0; i < selected_game_object->components.size(); ++i)
	{
		Component* component = selected_game_object->components[i];
		
		if (component == nullptr)
		{
			continue;
		}
		
		COMPONENT_TYPE type = component->GetType();	
		switch (type)
		{
		case COMPONENT_TYPE::TRANSFORM:	{ DrawTransformComponent((C_Transform*)component); }	break;
		case COMPONENT_TYPE::MESH:		{ DrawMeshComponent((C_Mesh*)component); }				break;
		case COMPONENT_TYPE::MATERIAL:	{ DrawMaterialComponent((C_Material*)component); }		break;
		case COMPONENT_TYPE::LIGHT:		{ DrawLightComponent((C_Light*)component); }			break;
		case COMPONENT_TYPE::CAMERA:	{ DrawCameraComponent((C_Camera*)component); }			break;
		case COMPONENT_TYPE::ANIMATOR:	{ DrawAnimatorComponent((C_Animator*)component); }		break;
		case COMPONENT_TYPE::ANIMATION: { DrawAnimationComponent((C_Animation*)component); }	break;
		}

		if (type == COMPONENT_TYPE::NONE)
		{
			LOG("[WARNING] Selected GameObject %s has a non-valid component!", selected_game_object->GetName());
		}
	}
}

void E_Inspector::DrawTransformComponent(C_Transform* c_transform)
{
	bool show = true;																				// Dummy bool to delete the component related with the collpsing header.
	if (ImGui::CollapsingHeader("Transform", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (c_transform != nullptr)
		{
			// --- IS ACTIVE ---
			bool transformIsActive = c_transform->IsActive();
			if (ImGui::Checkbox("Transform Is Active", &transformIsActive))
			{
				//transform->SetIsActive(transform_is_active);
				c_transform->SetIsActive(transformIsActive);
			}

			ImGui::Separator();

			// --- POSITION ---
			ImGui::Text("Position");

			ImGui::SameLine(100.0f);

			float3 position = c_transform->GetLocalPosition();
			if (ImGui::DragFloat3("P", (float*)&position, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
			{
				c_transform->SetLocalPosition(position);
			}

			// --- ROTATION ---
			ImGui::Text("Rotation");

			ImGui::SameLine(100.0f);

			/*float3 rotation = transform->GetLocalEulerRotation();
			if (ImGui::DragFloat3("R", (float*)&rotation, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
			{
				transform->SetLocalEulerRotation(rotation);
			}*/

			float3 rotation = c_transform->GetLocalEulerRotation() * RADTODEG;
			if (ImGui::DragFloat3("R", (float*)&rotation, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
			{	
				c_transform->SetLocalRotation(rotation * DEGTORAD);
			}

			// --- SCALE ---
			ImGui::Text("Scale");

			ImGui::SameLine(100.0f);

			float3 scale = c_transform->GetLocalScale();
			if (ImGui::DragFloat3("S", (float*)&scale, 0.05f, 0.0f, 0.0f, "%.3f", NULL))
			{
				c_transform->SetLocalScale(scale);
			}
		}

		if (!show)
		{
			LOG("[ERROR] Transform components cannot be deleted!");
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawMeshComponent(C_Mesh* c_mesh)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Mesh", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (c_mesh != nullptr)
		{
			// --- IS ACTIVE ---
			bool meshIsActive = c_mesh->IsActive();
			if (ImGui::Checkbox("Mesh Is Active", &meshIsActive))
			{
				c_mesh->SetIsActive(meshIsActive);
			}

			ImGui::Separator();

			// --- FILE PATH ---
			ImGui::Text("File:");	ImGui::SameLine(); ImGui::TextColored(Green.C_Array(), "%s", c_mesh->GetMeshFile());

			ImGui::Separator();

			// --- MESH DATA ---
			ImGui::TextColored(Cyan.C_Array(), "Mesh Data:");

			uint numVertices		= 0;
			uint numNormals		= 0;
			uint numTexCoords		= 0;
			uint numIndices		= 0;
			uint numBones			= 0;

			c_mesh->GetMeshData(numVertices, numNormals, numTexCoords, numIndices, numBones);

			ImGui::Text("Vertices:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "  %u", numVertices);
			ImGui::Text("Normals:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "   %u", numNormals);
			ImGui::Text("Tex Coords:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "%u", numTexCoords);
			ImGui::Text("Indices:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "   %u", numIndices);
			ImGui::Text("Bones: ");			ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "    %u",	numBones);

			ImGui::Separator();

			// --- DRAW MODE ---
			ImGui::TextColored(Cyan.C_Array(), "Draw Mode:");

			bool showWireframe		= c_mesh->GetShowWireframe();
			bool showBoundingBox	= c_mesh->GetShowBoundingBox();
			bool drawVertNormals	= c_mesh->GetDrawVertexNormals();
			bool drawFaceNormals	= c_mesh->GetDrawFaceNormals();

			if (ImGui::Checkbox("Show Wireframe", &showWireframe))				{ c_mesh->SetShowWireframe(showWireframe); }
			if (ImGui::Checkbox("Show Bounding Box", &showBoundingBox))		{ c_mesh->SetShowBoundingBox(showBoundingBox); }
			if (ImGui::Checkbox("Draw Vertex Normals", &drawVertNormals))		{ c_mesh->SetDrawVertexNormals(drawVertNormals); }
			if (ImGui::Checkbox("Draw Face Normals", &drawFaceNormals))		{ c_mesh->SetDrawFaceNormals(drawFaceNormals); }
		}
		else
		{
			LOG("[ERROR] Could not get the Mesh Component from %s Game Object!", c_mesh->GetOwner()->GetName());
		}

		if (!show)
		{
			componentToDelete				= c_mesh;
			showDeleteComponentPopup		= true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawMaterialComponent(C_Material* c_material)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Material", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (c_material != nullptr)
		{
			bool materialIsActive = c_material->IsActive();
			if (ImGui::Checkbox("Material Is Active", &materialIsActive))
			{
				c_material->SetIsActive(materialIsActive);
			}
			
			ImGui::Separator();

			// --- MATERIAL PATH ---
			ImGui::Text("File:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", c_material->GetTextureFile());

			ImGui::Separator();

			// --- MATERIAL COLOR & ALPHA ---
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Material Data:");

			Color color = c_material->GetMaterialColour();

			if (ImGui::ColorEdit3("Diffuse Color", (float*)&color, ImGuiColorEditFlags_NoAlpha))
			{
				c_material->SetMaterialColour(color);
			}

			if (ImGui::SliderFloat("Diffuse Alpha", (float*)&color.a, 0.0f, 1.0f, "%.3f"))
			{
				c_material->SetMaterialColour(color);
			}

			ImGui::Separator();

			// --- TEXTURE DATA ---
			DisplayTextureData(c_material);

			ImGui::Separator();

			// --- MAIN MAPS ---
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Main Maps:");

			if (ImGui::Combo("Textures(WIP)", &mapToDisplay, "Diffuse\0Specular\0Ambient\0Height\0Normal\0"))
			{
				LOG("[SCENE] Changed to map %d", mapToDisplay);
			}

			bool useCheckeredTex = c_material->UseDefaultTexture();
			if (ImGui::Checkbox("Use Default Texture", &useCheckeredTex))
			{
				c_material->SetUseDefaultTexture(useCheckeredTex);
			}

			// --- TEXTURE DISPLAY ---
			TextureDisplay(c_material);
		}
		else
		{
			LOG("[ERROR] Could not get the Material Component from %s Game Object!", c_material->GetOwner()->GetName());
		}

		if (!show)
		{
			componentToDelete				= c_material;
			showDeleteComponentPopup		= true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawLightComponent(C_Light* c_light)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Light", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (c_light != nullptr)
		{
			bool lightIsActive = c_light->IsActive();
			if (ImGui::Checkbox("Light Is Active", &lightIsActive))
			{
				c_light->SetIsActive(lightIsActive);
			}
			
			ImGui::Separator();
			
			ImGui::Text("WORK IN PROGRESS");
		}

		if (!show)
		{
			componentToDelete				= c_light;
			showDeleteComponentPopup		= true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawCameraComponent(C_Camera* c_camera)
{
	bool show = true;
	if (ImGui::CollapsingHeader("Camera", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (c_camera != nullptr)
		{
			bool cameraIsActive = c_camera->IsActive();
			if (ImGui::Checkbox("Camera Is Active", &cameraIsActive))
			{
				c_camera->SetIsActive(cameraIsActive);
			}

			ImGui::Separator();

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Camera Flags:");
			
			bool cameraIsCulling = c_camera->IsCulling();
			if (ImGui::Checkbox("Culling", &cameraIsCulling))
			{
				c_camera->SetIsCulling(cameraIsCulling);
			}

			bool cameraIsOrthogonal = c_camera->OrthogonalView();
			if (ImGui::Checkbox("Orthogonal", &cameraIsOrthogonal))
			{
				c_camera->SetOrthogonalView(cameraIsOrthogonal);
			} 

			bool frustumIsHidden = c_camera->FrustumIsHidden();
			if (ImGui::Checkbox("Hide Frustum", &frustumIsHidden))
			{
				c_camera->SetFrustumIsHidden(frustumIsHidden);
			}

			ImGui::Separator();

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Frustum Settings:");

			float nearPlaneDistance = c_camera->GetNearPlaneDistance();
			if (ImGui::SliderFloat("Near Plane", &nearPlaneDistance, 0.1f, 1000.0f, "%.3f", 0))
			{
				c_camera->SetNearPlaneDistance(nearPlaneDistance);
			}

			float farPlaneDistance = c_camera->GetFarPlaneDistance();
			if (ImGui::SliderFloat("Far Plane", &farPlaneDistance, 0.1f, 1000.0f, "%.3f", 0))
			{
				c_camera->SetFarPlaneDistance(farPlaneDistance);
			}
			
			int fov			= (int)c_camera->GetVerticalFOV();
			uint minFov	= 0;
			uint maxFov	= 0;
			c_camera->GetMinMaxFOV(minFov, maxFov);
			if (ImGui::SliderInt("FOV", &fov, minFov, maxFov, "%d"))
			{
				c_camera->SetVerticalFOV((float)fov);
			}
			
			ImGui::Separator();

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Camera Selection:");

			if (ImGui::Button("Set as Current Camera"))
			{
				App->editor->SetCurrentCameraThroughEditor(c_camera);
			}

			if (ImGui::Button("Return to Master Camera"))
			{
				App->editor->SetMasterCameraThroughEditor();
			}
		}

		if (!show)
		{
			componentToDelete				= c_camera;
			showDeleteComponentPopup		= true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::DrawAnimatorComponent(C_Animator* c_animator)								// TODO: Segment this Method in Multiple Smaller Methods.
{
	bool show = true;
	if (ImGui::CollapsingHeader("Animator", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (c_animator != nullptr)
		{
			bool animationIsActive	= c_animator->IsActive();
			if (ImGui::Checkbox("Is Active", &animationIsActive))							{ c_animator->SetIsActive(animationIsActive); }

			ImGui::Separator();

			// --- ANIMATOR VARIABLES
			static int selectedClip			= 0;
			std::string clipNamesString		= c_animator->GetClipNamesAsString();

			float speed							= c_animator->GetPlaybackSpeed();
			float minSpeed						= 0.1f;
			float maxSpeed						= 10.0f;
			
			bool interpolate					= c_animator->GetInterpolate();
			bool loopAnimation					= c_animator->GetLoopAnimation();
			bool playOnStart					= c_animator->GetPlayOnStart();
			bool cameraCulling					= c_animator->GetCameraCulling();
			bool showBones						= c_animator->GetShowBones();

			// -- CURRENT CLIP VARIABLES
			AnimatorClip* currentClip			= c_animator->GetCurrentClip();

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
			std::string animationNames			= c_animator->GetAnimationNamesAsString();

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
			std::vector<std::string> clipNames = c_animator->GetClipNamesAsVector();

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
						c_animator->SetCurrentClipByIndex((uint)selectedClip);
					}

					if (ImGui::Button("Play"))									{ c_animator->Play(); }		ImGui::SameLine();
					if (ImGui::Button("Pause"))									{ c_animator->Pause(); }	ImGui::SameLine();
					if (ImGui::Button("Step"))									{ c_animator->Step(); }		ImGui::SameLine();
					if (ImGui::Button("Stop"))									{ c_animator->Stop(); }

					if (ImGui::SliderFloat("Playback Speed", &speed, minSpeed, maxSpeed, "X %.3f", 0)) { c_animator->SetPlaybackSpeed(speed); }

					if (ImGui::Checkbox("Interpolate", &interpolate))			{ c_animator->SetInterpolate(interpolate); }
					if (ImGui::Checkbox("Loop Animation", &loopAnimation))		{ c_animator->SetLoopAnimation(loopAnimation); }
					if (ImGui::Checkbox("Play On Start", &playOnStart))		{ c_animator->SetPlayOnStart(playOnStart); }
					if (ImGui::Checkbox("Camera Culling", &cameraCulling))		{ c_animator->SetCameraCulling(cameraCulling); }
					if (ImGui::Checkbox("Show Bones", &showBones))				{ c_animator->SetShowBones(showBones); }

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

					if (ImGui::Button("Previous Keyframe"))		{ c_animator->StepToPrevKeyframe(); }	ImGui::SameLine(150.0f);
					if (ImGui::Button("Next Keyframe"))			{ c_animator->StepToNextKeyframe(); }
					if (ImGui::Button("Refresh Bone Display"))	{ c_animator->RefreshBoneDisplay(); }

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
							success = c_animator->AddClip(AnimatorClip(c_animator->GetAnimationByIndex((uint)selectedAnimation), newClipName, newClipStart, newClipEnd, loop));
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
			componentToDelete				= c_animator;
			showDeleteComponentPopup		= true;
		}
		
		ImGui::Separator();
	}
}

void E_Inspector::DrawAnimationComponent(C_Animation* c_animation)
{
	static bool show = true;
	if (ImGui::CollapsingHeader("Animation", &show, ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (c_animation != nullptr)
		{
			bool isActive = c_animation->IsActive();
			if (ImGui::Checkbox("Is Active", &isActive)) { c_animation->SetIsActive(isActive); }

			ImGui::Separator();

			ImGui::TextColored(Cyan.C_Array(), "Animation Settings:");

			ImGui::Separator();
		}

		if (!show)
		{
			componentToDelete				= c_animation;
			showDeleteComponentPopup		= true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::AddComponentCombo(GameObject* selected_game_object)
{
	ImGui::Combo("##", &componentType, "Add Component\0Transform\0Mesh\0Material\0Light\0Camera\0Animator\0Animation");

	ImGui::SameLine();

	if ((ImGui::Button("ADD")))
	{ 
		if (componentType != (int)COMPONENT_TYPE::NONE)
		{
			selected_game_object->CreateComponent((COMPONENT_TYPE)componentType);
		}
	}
}

void E_Inspector::DeleteComponentPopup(GameObject* selected_game_object)
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
			selected_game_object->DeleteComponent(componentToDelete);
			
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

void E_Inspector::DisplayTextureData(C_Material* c_material)
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

	c_material->GetTextureInfo(id, width, height, depth, bpp, size, format, compressed);

	ImGui::Text("Texture ID:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%u", id);
	ImGui::Text("Width:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "     %upx", width);
	ImGui::Text("Height:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %upx", height);
	ImGui::Text("Depth:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "     %u", depth);
	ImGui::Text("Bpp:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "       %uB", bpp);
	ImGui::Text("Size:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "      %uB", size);
	ImGui::Text("Format:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %s", format.c_str());
	ImGui::Text("Compressed:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", compressed ? "True" : "False");
}

void E_Inspector::TextureDisplay(C_Material* c_material)
{
	ImTextureID texId		= 0;
	ImVec2 displaySize		= { ImGui::GetWindowWidth() * 0.925f , ImGui::GetWindowWidth() * 0.925f };		// Display Size will be 7.5% smaller than the Window Width.
	ImVec4 tint				= { 1.0f, 1.0f, 1.0f, 1.0f };
	ImVec4 borderColor		= { 0.0f, 1.0f, 0.0f, 1.0f };

	if (c_material->UseDefaultTexture())
	{
		texId = (ImTextureID)App->renderer->GetDebugTextureID();
		//tex_id = (ImTextureID)App->renderer->GetSceneRenderTexture();
	}
	else
	{
		texId = (ImTextureID)c_material->GetTextureID();
	}

	if (texId != 0)
	{
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Texture Display:");

		ImGui::Spacing();

		ImGui::Image(texId, displaySize, ImVec2(1.0f, 0.0f), ImVec2(0.0f, 1.0f), tint, borderColor);			// ImGui has access to OpenGL's buffers, so only the Texture Id is required.
	}
}