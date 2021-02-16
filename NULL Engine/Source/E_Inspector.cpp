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
show_delete_component_popup	(false),
component_type				(0),
map_to_display				(0),
component_to_delete			(nullptr)
{

}

E_Inspector::~E_Inspector()
{
	component_to_delete = nullptr;
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

		if (show_delete_component_popup)
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
	bool game_object_is_active = selected_game_object->IsActive();
	if (ImGui::Checkbox("Is Active", &game_object_is_active))
	{
		selected_game_object->SetIsActive(game_object_is_active);
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
	bool is_static = true;
	if (ImGui::Checkbox("Is Static", &is_static))
	{
		selected_game_object->SetIsStatic(is_static);
	}

	// --- TAG ---
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.33f);
	static char tag_combo[64] = { "Untagged\0Work\0In\0Progress" };
	static int current_tag = 0;
	ImGui::Combo("Tag", &current_tag, tag_combo);

	ImGui::SameLine(218.0f);

	// --- LAYER ---
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.33f);
	static char layer_combo[64] = { "Default\0Work\0In\0Progress" };
	static int current_layer = 0;
	ImGui::Combo("Layer", &current_layer, layer_combo);

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
			bool transform_is_active = c_transform->IsActive();
			if (ImGui::Checkbox("Transform Is Active", &transform_is_active))
			{
				//transform->SetIsActive(transform_is_active);
				c_transform->SetIsActive(transform_is_active);
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
			bool mesh_is_active = c_mesh->IsActive();
			if (ImGui::Checkbox("Mesh Is Active", &mesh_is_active))
			{
				c_mesh->SetIsActive(mesh_is_active);
			}

			ImGui::Separator();

			// --- FILE PATH ---
			ImGui::Text("File:");	ImGui::SameLine(); ImGui::TextColored(Green.C_Array(), "%s", c_mesh->GetMeshFile());

			ImGui::Separator();

			// --- MESH DATA ---
			ImGui::TextColored(Cyan.C_Array(), "Mesh Data:");

			uint num_vertices		= 0;
			uint num_normals		= 0;
			uint num_tex_coords		= 0;
			uint num_indices		= 0;
			uint num_bones			= 0;

			c_mesh->GetMeshData(num_vertices, num_normals, num_tex_coords, num_indices, num_bones);

			ImGui::Text("Vertices:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "  %u", num_vertices);
			ImGui::Text("Normals:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "   %u", num_normals);
			ImGui::Text("Tex Coords:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "%u", num_tex_coords);
			ImGui::Text("Indices:");		ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "   %u", num_indices);
			ImGui::Text("Bones: ");			ImGui::SameLine();		ImGui::TextColored(Yellow.C_Array(), "    %u",	num_bones);

			ImGui::Separator();

			// --- DRAW MODE ---
			ImGui::TextColored(Cyan.C_Array(), "Draw Mode:");

			bool show_wireframe		= c_mesh->GetShowWireframe();
			bool show_bounding_box	= c_mesh->GetShowBoundingBox();
			bool draw_vert_normals	= c_mesh->GetDrawVertexNormals();
			bool draw_face_normals	= c_mesh->GetDrawFaceNormals();

			if (ImGui::Checkbox("Show Wireframe", &show_wireframe))				{ c_mesh->SetShowWireframe(show_wireframe); }
			if (ImGui::Checkbox("Show Bounding Box", &show_bounding_box))		{ c_mesh->SetShowBoundingBox(show_bounding_box); }
			if (ImGui::Checkbox("Draw Vertex Normals", &draw_vert_normals))		{ c_mesh->SetDrawVertexNormals(draw_vert_normals); }
			if (ImGui::Checkbox("Draw Face Normals", &draw_face_normals))		{ c_mesh->SetDrawFaceNormals(draw_face_normals); }
		}
		else
		{
			LOG("[ERROR] Could not get the Mesh Component from %s Game Object!", c_mesh->GetOwner()->GetName());
		}

		if (!show)
		{
			component_to_delete				= c_mesh;
			show_delete_component_popup		= true;
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
			bool material_is_active = c_material->IsActive();
			if (ImGui::Checkbox("Material Is Active", &material_is_active))
			{
				c_material->SetIsActive(material_is_active);
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

			if (ImGui::Combo("Textures(WIP)", &map_to_display, "Diffuse\0Specular\0Ambient\0Height\0Normal\0"))
			{
				LOG("[SCENE] Changed to map %d", map_to_display);
			}

			bool use_checkered_tex = c_material->UseDefaultTexture();
			if (ImGui::Checkbox("Use Default Texture", &use_checkered_tex))
			{
				c_material->SetUseDefaultTexture(use_checkered_tex);
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
			component_to_delete				= c_material;
			show_delete_component_popup		= true;
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
			bool light_is_active = c_light->IsActive();
			if (ImGui::Checkbox("Light Is Active", &light_is_active))
			{
				c_light->SetIsActive(light_is_active);
			}
			
			ImGui::Separator();
			
			ImGui::Text("WORK IN PROGRESS");
		}

		if (!show)
		{
			component_to_delete				= c_light;
			show_delete_component_popup		= true;
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
			bool camera_is_active = c_camera->IsActive();
			if (ImGui::Checkbox("Camera Is Active", &camera_is_active))
			{
				c_camera->SetIsActive(camera_is_active);
			}

			ImGui::Separator();

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Camera Flags:");
			
			bool camera_is_culling = c_camera->IsCulling();
			if (ImGui::Checkbox("Culling", &camera_is_culling))
			{
				c_camera->SetIsCulling(camera_is_culling);
			}

			bool camera_is_orthogonal = c_camera->OrthogonalView();
			if (ImGui::Checkbox("Orthogonal", &camera_is_orthogonal))
			{
				c_camera->SetOrthogonalView(camera_is_orthogonal);
			} 

			bool frustum_is_hidden = c_camera->FrustumIsHidden();
			if (ImGui::Checkbox("Hide Frustum", &frustum_is_hidden))
			{
				c_camera->SetFrustumIsHidden(frustum_is_hidden);
			}

			ImGui::Separator();

			ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Frustum Settings:");

			float near_plane_distance = c_camera->GetNearPlaneDistance();
			if (ImGui::SliderFloat("Near Plane", &near_plane_distance, 0.1f, 1000.0f, "%.3f", 0))
			{
				c_camera->SetNearPlaneDistance(near_plane_distance);
			}

			float far_plane_distance = c_camera->GetFarPlaneDistance();
			if (ImGui::SliderFloat("Far Plane", &far_plane_distance, 0.1f, 1000.0f, "%.3f", 0))
			{
				c_camera->SetFarPlaneDistance(far_plane_distance);
			}
			
			int fov			= (int)c_camera->GetVerticalFOV();
			uint min_fov	= 0;
			uint max_fov	= 0;
			c_camera->GetMinMaxFOV(min_fov, max_fov);
			if (ImGui::SliderInt("FOV", &fov, min_fov, max_fov, "%d"))
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
			component_to_delete				= c_camera;
			show_delete_component_popup		= true;
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
			bool animation_is_active	= c_animator->IsActive();
			if (ImGui::Checkbox("Is Active", &animation_is_active))							{ c_animator->SetIsActive(animation_is_active); }

			ImGui::Separator();

			// --- ANIMATOR VARIABLES
			static int selected_clip			= 0;
			std::string clip_names_string		= c_animator->GetClipNamesAsString();

			float speed							= c_animator->GetPlaybackSpeed();
			float min_speed						= 0.1f;
			float max_speed						= 10.0f;
			
			bool interpolate					= c_animator->GetInterpolate();
			bool loop_animation					= c_animator->GetLoopAnimation();
			bool play_on_start					= c_animator->GetPlayOnStart();
			bool camera_culling					= c_animator->GetCameraCulling();
			bool show_bones						= c_animator->GetShowBones();

			// -- CURRENT CLIP VARIABLES
			AnimatorClip* current_clip			= c_animator->GetCurrentClip();

			if (current_clip == nullptr)
			{
				current_clip = &AnimatorClip();
			}

			const char* animation_name			= current_clip->GetAnimationName();
			float animation_ticks_per_second	= current_clip->GetAnimationTicksPerSecond();
			float animation_duration			= current_clip->GetAnimationDuration();

			const char* current_clip_name		= current_clip->GetName();
			uint current_clip_start				= current_clip->GetStart();
			uint current_clip_end				= current_clip->GetEnd();
			float current_clip_duration			= current_clip->GetDuration();
			bool current_clip_loop				= current_clip->IsLooped();

			float clip_time						= current_clip->GetClipTime();
			float clip_frame					= current_clip->GetClipFrame();
			uint clip_ticks						= current_clip->GetClipTick();

			// --- NEW CLIP VARIABLES
			static int selected_animation		= 0;
			std::string animation_names			= c_animator->GetAnimationNamesAsString();

			static char new_clip_name[128]		= "Enter Clip Name";
			ImGuiInputTextFlags input_txt_flags	= ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;

			static int new_clip_start			= 0;
			static int new_clip_end				= (int)animation_duration;
			int new_clip_min					= 0;
			int new_clip_max					= (int)animation_duration;
			static bool loop					= false;

			static bool success					= false;																			// --- TODO: Transform into non-static variables later.
			static bool text_timer_running		= false;																			//
			static float text_timer				= 0.0f;																				//
			static float text_duration			= 2.5f;																				// ----------------------------------------------------
			
			// --- EXISTING CLIPS VARIABLES
			std::vector<std::string> clip_names = c_animator->GetClipNamesAsVector();

			// --- DISPLAY
			if (ImGui::BeginTabBar("AnimatorTabBar", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("Settings & Controls"))
				{
					// --- ANIMATOR SETTINGS
					ImGui::TextColored(Cyan.C_Array(), "Animation Settings");

					if (ImGui::Combo("Select Clip", &selected_clip, clip_names_string.c_str()))
					{
						char selected_name = clip_names_string[selected_clip];
						c_animator->SetCurrentClipByIndex((uint)selected_clip);
					}

					if (ImGui::Button("Play"))									{ c_animator->Play(); }		ImGui::SameLine();
					if (ImGui::Button("Pause"))									{ c_animator->Pause(); }	ImGui::SameLine();
					if (ImGui::Button("Step"))									{ c_animator->Step(); }		ImGui::SameLine();
					if (ImGui::Button("Stop"))									{ c_animator->Stop(); }

					if (ImGui::SliderFloat("Playback Speed", &speed, min_speed, max_speed, "X %.3f", 0)) { c_animator->SetPlaybackSpeed(speed); }

					if (ImGui::Checkbox("Interpolate", &interpolate))			{ c_animator->SetInterpolate(interpolate); }
					if (ImGui::Checkbox("Loop Animation", &loop_animation))		{ c_animator->SetLoopAnimation(loop_animation); }
					if (ImGui::Checkbox("Play On Start", &play_on_start))		{ c_animator->SetPlayOnStart(play_on_start); }
					if (ImGui::Checkbox("Camera Culling", &camera_culling))		{ c_animator->SetCameraCulling(camera_culling); }
					if (ImGui::Checkbox("Show Bones", &show_bones))				{ c_animator->SetShowBones(show_bones); }

					ImGui::Separator();

					// --- ANIMATOR STATS
					ImGui::TextColored(Cyan.C_Array(), "Animation Stats");

					ImGui::Text("Name:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "             %s",		animation_name);
					ImGui::Text("Ticks Per Second:");	ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), " %.3f",				animation_ticks_per_second);
					ImGui::Text("Duration:");			ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "         %.3f",		animation_duration);

					ImGui::Separator();

					ImGui::TextColored(Cyan.C_Array(), "Clip Stats");

					ImGui::Text("Name:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "             %s",		current_clip_name);
					ImGui::Text("Time:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "             %.3f",	clip_time);
					ImGui::Text("Frame:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "            %.3f",	clip_frame);
					ImGui::Text("Tick:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "		     %u",		clip_ticks);
					ImGui::Text("Range:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "            %u - %u", current_clip_start, current_clip_end);
					ImGui::Text("Duration:");			ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "         %.3f",		current_clip_duration);
					ImGui::Text("Loop:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), "             %s",		(current_clip_loop) ? "True" : "False");

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

					ImGui::Combo("Select Animation", &selected_animation, animation_names.c_str());
					ImGui::InputText("Clip Name", new_clip_name, IM_ARRAYSIZE(new_clip_name), input_txt_flags);
					ImGui::SliderInt("Clip Start", &new_clip_start, new_clip_min, new_clip_max);
					ImGui::SliderInt("Clip End", &new_clip_end, new_clip_min, new_clip_max);
					ImGui::Checkbox("Loop Clip", &loop);

					if (new_clip_start > new_clip_end) { new_clip_end = new_clip_start; };

					if (ImGui::Button("Create")) 
					{ 
						if (!App->play)
						{
							success = c_animator->AddClip(AnimatorClip(c_animator->GetAnimationByIndex((uint)selected_animation), new_clip_name, new_clip_start, new_clip_end, loop));
							text_timer_running = true;
						}
						else
						{
							ImGui::SameLine();
							ImGui::TextColored(Red.C_Array(), "Cannot Create Clips While in Game Mode!");
						}
					}

					if (text_timer_running)
					{	
						ImGui::SameLine();
						
						if (success)
						{
							static std::string new_clip_name_str = new_clip_name;
							ImGui::TextColored(Green.C_Array(), "Successfully Created Clip { %s }", new_clip_name_str.c_str());

							strcpy_s(new_clip_name, 128, "Enter Clip Name");																// --- Re-setting the New Clip Parameters
							new_clip_start	= 0;																							// 
							new_clip_end	= (int)animation_duration;																		// 
							loop			= false;																						// --------------------------------------
						}
						else
						{
							ImGui::TextColored(Red.C_Array(), "A clip with the same name already exists!");
						}

						text_timer += Time::Real::GetDT();
						if (text_timer > text_duration)
						{
							text_timer_running = false;
							text_timer = 0.0f;
						}
					}

					ImGui::EndChild();
					ImGui::PopStyleVar();

					ImGui::Separator();
					ImGui::Separator();

					ImGui::TextColored(Cyan.C_Array(), "Existing Clips");

					for (uint i = 0; i < clip_names.size(); ++i)
					{	
						if (ImGui::TreeNodeEx(clip_names[i].c_str(), ImGuiTreeNodeFlags_Bullet))
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
			component_to_delete				= c_animator;
			show_delete_component_popup		= true;
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
			bool is_active = c_animation->IsActive();
			if (ImGui::Checkbox("Is Active", &is_active)) { c_animation->SetIsActive(is_active); }

			ImGui::Separator();

			ImGui::TextColored(Cyan.C_Array(), "Animation Settings:");

			ImGui::Separator();
		}

		if (!show)
		{
			component_to_delete				= c_animation;
			show_delete_component_popup		= true;
		}

		ImGui::Separator();
	}
}

void E_Inspector::AddComponentCombo(GameObject* selected_game_object)
{
	ImGui::Combo("##", &component_type, "Add Component\0Transform\0Mesh\0Material\0Light\0Camera\0Animator\0Animation");

	ImGui::SameLine();

	if ((ImGui::Button("ADD")))
	{ 
		if (component_type != (int)COMPONENT_TYPE::NONE)
		{
			selected_game_object->CreateComponent((COMPONENT_TYPE)component_type);
		}
	}
}

void E_Inspector::DeleteComponentPopup(GameObject* selected_game_object)
{
	std::string title	=	"Delete ";																// Generating the specific string for the Popup title.
	title				+=	component_to_delete->GetNameFromType();									// The string will be specific to the component to delete.
	title				+=	" Component?";															// -------------------------------------------------------
	
	ImGui::OpenPopup(title.c_str());
	
	bool show = true;																				// Dummy bool to close the popup without having to click the "CONFIRM" or "CANCEL" Buttons.
	if (ImGui::BeginPopupModal(title.c_str(), &show))
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.25f));
		if (ImGui::Button("CONFIRM"))																// CONFIRM Button. Will delete the component to delete.
		{
			selected_game_object->DeleteComponent(component_to_delete);
			
			component_to_delete				= nullptr;
			show_delete_component_popup		= false;

			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor();

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.25f));
		if (ImGui::Button("CANCEL"))																// CANCEL Button. Will close the Popup.
		{
			component_to_delete				= nullptr;
			show_delete_component_popup		= false;
			
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleColor();

		if (!show)																					// Popup cross. Will close the Popup. UX.
		{
			component_to_delete				= nullptr;
			show_delete_component_popup		= false;
			
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
	ImTextureID tex_id		= 0;
	ImVec2 display_size		= { ImGui::GetWindowWidth() * 0.925f , ImGui::GetWindowWidth() * 0.925f };		// Display Size will be 7.5% smaller than the Window Width.
	ImVec4 tint				= { 1.0f, 1.0f, 1.0f, 1.0f };
	ImVec4 border_color		= { 0.0f, 1.0f, 0.0f, 1.0f };

	if (c_material->UseDefaultTexture())
	{
		tex_id = (ImTextureID)App->renderer->GetDebugTextureID();
		//tex_id = (ImTextureID)App->renderer->GetSceneRenderTexture();
	}
	else
	{
		tex_id = (ImTextureID)c_material->GetTextureID();
	}

	if (tex_id != 0)
	{
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Texture Display:");

		ImGui::Spacing();

		ImGui::Image(tex_id, display_size, ImVec2(1.0f, 0.0f), ImVec2(0.0f, 1.0f), tint, border_color);			// ImGui has access to OpenGL's buffers, so only the Texture Id is required.
	}
}