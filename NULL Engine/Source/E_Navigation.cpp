#include "Profiler.h"
#include "E_Navigation.h"

#include <queue>

#include "Application.h"
#include "M_Detour.h"
#include "GameObject.h"
#include "Component.h"
#include "C_Mesh.h"
#include "M_Scene.h"
#include "R_NavMesh.h"

#include "EngineApplication.h"
#include "M_Recast.h"

#include "RecastNavigation/Detour/Include/DetourNavMesh.h"

E_Navigation::E_Navigation() : EditorPanel("Navigation")
{

}

E_Navigation::~E_Navigation()
{

}

bool E_Navigation::Draw(ImGuiIO& io)
{
	OPTICK_CATEGORY("E_Hierarchy Draw", Optick::Category::Editor)

	ImGui::Begin(GetName());

	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	if (ImGui::BeginTabBar("NavigationTabBar", tab_bar_flags)) {
		if (ImGui::BeginTabItem("Bake")) {
			ImGui::Checkbox("Draw NavMesh", &App->detour->debugDraw);
			ImGui::Separator();
			ImGui::Text("Baked Agent Size");
			float agentRadius = App->detour->agentRadius;
			float agentHeight = App->detour->agentHeight;
			float agentMaxClimb = App->detour->agentMaxClimb;
			float maxSlopeAngle = App->detour->maxSlopeAngle;
			float stepHeight = App->detour->stepHeight;
	
			if (ImGui::DragFloat("Agent Radius", &agentRadius, 0.25f, 0.01f, 9999999999.0f, "%.2f", 1.0f)) {
				App->detour->agentRadius = agentRadius;
			}
			if (ImGui::DragFloat("Agent Height", &agentHeight, 0.25f, 0.02f, 9999999999.0f, "%.2f", 1.0f)) {
				App->detour->agentHeight = agentHeight;
			}
			if (ImGui::DragFloat("Max Agent Climb", &agentMaxClimb, 0.0f, 0.02f, 60.0f, "%.2f", 1.0f)) {
				App->detour->agentMaxClimb = agentMaxClimb;
			}
			if (ImGui::SliderFloat("Max Slope Angle", &maxSlopeAngle, 0.0f, 60.0f, "%.1f")) {
				App->detour->maxSlopeAngle = maxSlopeAngle;
			}
			if (ImGui::DragFloat("Step Height", &stepHeight, 0.25f, 0.02f, 9999999999.0f, "%.2f", 1.0f)) {
				App->detour->stepHeight = stepHeight;
			}

			ImGui::Text("Build Tiled Mesh"); ImGui::SameLine();
			ImGui::Checkbox("##tiledMesh", &App->detour->buildTiledMesh);

			if (ImGui::TreeNode("Advanced Options")) {
				ImGui::Text("Manual Voxel Size"); ImGui::SameLine();
				ImGui::Checkbox("##manualVoxelSize", &manualVoxel);
				if (manualVoxel)
					ImGui::DragFloat("Voxel Size##dragFloat", &App->detour->voxelSize, 0.25f, 0.02f, 9999999999.0f, "%.2f", 1.0f);
				else {
					App->detour->voxelSize = App->detour->agentRadius / 3.0f;
					float voxelSize = App->detour->voxelSize;
					ImGui::DragFloat("Voxel Size##inputText", &voxelSize, 0.0f, 0.0f, 0.0f, "%.2f");
				}
				ImGui::DragFloat("Voxel Height", &App->detour->voxelHeight, 0.25f, 0.02f, 9999999999.0f, "%.2f", 1.0f);
				ImGui::DragFloat("Region Minimum Size", &App->detour->regionMinSize, 0.25f, 0.02f, 9999999999.0f, "%.2f", 1.0f);
				ImGui::DragFloat("Region Merge Size", &App->detour->regionMergeSize, 0.25f, 0.02f, 9999999999.0f, "%.2f", 1.0f);
				ImGui::DragFloat("Edge Maximum Length", &App->detour->edgeMaxLen, 0.25f, 0.02f, 9999999999.0f, "%.2f", 1.0f);
				ImGui::DragFloat("Edge Maximum Error", &App->detour->edgeMaxError, 0.25f, 0.02f, 9999999999.0f, "%.2f", 1.0f);
				ImGui::DragFloat("Vertices Per Polygon", &App->detour->vertsPerPoly, 0.25f, 0.02f, 9999999999.0f, "%.2f", 1.0f);
				ImGui::DragFloat("Detail Sample Distance", &App->detour->detailSampleDist, 0.25f, 0.02f, 9999999999.0f, "%.2f", 1.0f);
				ImGui::DragFloat("Detail Sample Max. Error", &App->detour->detailSampleMaxError, 0.25f, 0.02f, 9999999999.0f, "%.2f", 1.0f);
				ImGui::TreePop();
			}
			ImGui::Separator();
			if (App->detour->getNavMeshResource() != nullptr) {
				if (ImGui::Button("Clear")) {
					App->detour->deleteNavMesh();
				}
				ImGui::SameLine();
			}
			if (ImGui::Button("Bake")) {
				EngineApp->recast->BuildNavMesh();
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Object")) {
			GameObject* selected = App->scene->GetSelectedGameObject();
			if (selected != nullptr && selected->GetComponent<C_Mesh>() != nullptr) {
				ImGui::Text(selected->GetName());
				ImGui::Separator();
				ImGui::Text("Navigation Static"); 
				ImGui::SameLine();
				isNavigable = selected->isNavigable;
				if (ImGui::Checkbox("##isNavigableCheckbox", &isNavigable)) {
					selected->isNavigable = isNavigable;
					if (isNavigable)
						EngineApp->recast->AddGO(selected);
					else
						EngineApp->recast->DeleteGO(selected);
				}
				if (selected->isNavigable) {
					ImGui::Text("Navigation Area"); ImGui::SameLine();
					if (ImGui::BeginCombo("##areaCombo", App->detour->areaNames[selected->navigationArea])) {
						for (int i = 0; i < BE_DETOUR_TOTAL_AREAS; ++i) {
							std::string areaName = App->detour->areaNames[i];
							if (areaName != "") {
								if (ImGui::Selectable(App->detour->areaNames[i], i == selected->navigationArea)) {
									selected->navigationArea = i;
								}
							}
						}
						ImGui::EndCombo();
					}
				}
			}
			else
				ImGui::Text("Select one mesh from the scene.");

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();

	return true;
}

bool E_Navigation::CleanUp()
{
	return true;
}