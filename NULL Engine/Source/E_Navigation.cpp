#include "Profiler.h"
#include "E_Navigation.h"

#include <queue>

#include "Application.h"
#include "M_Detour.h"
#include "GameObject.h"
#include "Component.h"
#include "C_Mesh.h"
#include "M_Scene.h"

#include "EngineApplication.h"
#include "M_Recast.h"


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
		if (ImGui::BeginTabItem("Areas")) {
			ImGui::Columns(3, "mycolumns3", false);
			ImGui::SetColumnWidth(0, 100);
			ImGui::SetColumnWidth(2, 100);
			ImGui::SetColumnOffset(1, 105);
			float middleColumn = math::Max<float>(ImGui::GetWindowWidth() - 210, 100);
			ImGui::SetColumnWidth(1, middleColumn);
			ImGui::SetColumnOffset(2, 105 + middleColumn);

			//Indeces
			ImGui::NextColumn();
			ImGui::Text("Name");
			ImGui::NextColumn();
			ImGui::Text("Cost");
			ImGui::Separator();

			//Built-ins
			ImGui::NextColumn();
			ImGui::Text("Built-in 0");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(ImGui::GetColumnWidth() - 10);
			ImGui::InputText("##name_bi0", App->detour->areaNames[0], 100, ImGuiInputTextFlags_ReadOnly);
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(ImGui::GetColumnWidth() - 10);
			float areaCost = App->detour->areaCosts[0];
			if (ImGui::InputFloat("##cost_bi0", &areaCost))
				App->detour->setAreaCost(0, areaCost);

			ImGui::NextColumn();
			ImGui::Text("Built-in 1");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(ImGui::GetColumnWidth() - 10);
			ImGui::InputText("##name_bi1", App->detour->areaNames[1], 100, ImGuiInputTextFlags_ReadOnly);
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(ImGui::GetColumnWidth() - 10);
			char buf[10];
			sprintf_s(buf, "1");
			ImGui::InputText("##cost_bi1", buf, 10, ImGuiInputTextFlags_ReadOnly);

			ImGui::NextColumn();
			ImGui::Text("Built-in 2");
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(ImGui::GetColumnWidth() - 10);
			ImGui::InputText("##name_bi2", App->detour->areaNames[2], 100, ImGuiInputTextFlags_ReadOnly);
			ImGui::NextColumn();
			ImGui::SetNextItemWidth(ImGui::GetColumnWidth() - 10);
			areaCost = App->detour->areaCosts[2];
			if (ImGui::InputFloat("##cost_bi2", &areaCost))
				App->detour->setAreaCost(2, areaCost);

			std::string catname = "User ";
			std::string namelabel = "##name_us";
			std::string costlabel = "##cost_us";

			for (int i = 3; i < BE_DETOUR_TOTAL_AREAS; ++i) {
				ImGui::NextColumn();
				ImGui::Text((catname + std::to_string(i)).c_str());
				ImGui::NextColumn();
				ImGui::SetNextItemWidth(ImGui::GetColumnWidth() - 10);
				ImGui::InputText((namelabel + std::to_string(i)).c_str(), App->detour->areaNames[i], 100);
				ImGui::NextColumn();
				ImGui::SetNextItemWidth(ImGui::GetColumnWidth() - 10);
				areaCost = App->detour->areaCosts[i];
				if (ImGui::InputFloat((costlabel + std::to_string(i)).c_str(), &areaCost))
					App->detour->setAreaCost(i, areaCost);
			}



			ImGui::Columns(1);

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Bake")) {
			ImGui::Text("Baked Agent Size");
			bool updateValues;
			if (ImGui::DragFloat("Agent Radius", &App->detour->agentRadius, 0.25f, 0.01f, 9999999999.0f, "%.2f", 1.0f)) {
			}
			if (ImGui::DragFloat("Agent Height", &App->detour->agentHeight, 0.25f, 0.02f, 9999999999.0f, "%.2f", 1.0f)) {
			}
			if (ImGui::SliderFloat("Max Slope", &App->detour->maxSlope, 0.0f, 60.0f, "%.1f")) {
			}
			if (ImGui::DragFloat("Step Height", &App->detour->stepHeight, 0.25f, 0.02f, 9999999999.0f, "%.2f", 1.0f)) {
			}

			if (ImGui::TreeNode("Advanced")) {
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
				ImGui::Text("Build Tiled Mesh"); ImGui::SameLine();
				ImGui::Checkbox("##tiledMesh", &App->detour->buildTiledMesh);
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
			//MYTODO this should probably be a MeshRenderer rather than a mesh, but Cameras GO have MeshRenderers
			uint selected_GOs = App->scene->GetGameObjects()->size();
			if (selected_GOs <= 1) {
				GameObject* selected = App->scene->GetSelectedGameObject();
				if (selected != nullptr && selected->GetComponent<C_Mesh>() != nullptr) {
					ImGui::Text(selected->GetName());
					ImGui::Separator();
					ImGui::Text("Navigation Static"); ImGui::SameLine();
					if (ImGui::BeginPopup("Change Static Flags")) {
						std::string enable;
						if (popupNavigationFlag)
							enable = "enable";
						else
							enable = "disable";

						ImGui::Text(std::string("Do you want to " + enable + " the Navigation Static flag for al the child objects as well?").c_str());
						if (ImGui::Button("Yes, change children")) {
							std::queue<GameObject*> childs;
							childs.push(selected);

							while (!childs.empty()) {
								GameObject* current_child = childs.front();
								childs.pop();

								//We add all of its childs
								for (uint i = 0; i < current_child->childs.size(); i++)
									childs.push(current_child->childs[i]);

								//We change the value of static
								if (current_child->GetComponent<C_Mesh>() != nullptr && current_child->isNavigable != popupNavigationFlag) {
									current_child->isNavigable = popupNavigationFlag;
									if (popupNavigationFlag)
										EngineApp->recast->AddGO(current_child);
									else
										EngineApp->recast->DeleteGO(current_child);
								}
							}
							ImGui::CloseCurrentPopup();
						}
						ImGui::SameLine();
						if (ImGui::Button("No, this object only")) {
							selected->isNavigable = popupNavigationFlag;
							if (popupNavigationFlag)
								EngineApp->recast->AddGO(selected);
							else
								EngineApp->recast->DeleteGO(selected);
							ImGui::CloseCurrentPopup();
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel"))
							ImGui::CloseCurrentPopup();

						ImGui::EndPopup();
					}
					isNavigable = selected->isNavigable;
					if (ImGui::Checkbox("##isNavigableCheckbox", &isNavigable)) {
						if (selected->childs.size() > 0) {
							popupNavigationFlag = isNavigable;
							ImGui::OpenPopup("Change Static Flags");
						}
						else {
							selected->isNavigable = isNavigable;
							if (isNavigable)
								EngineApp->recast->AddGO(selected);
							else
								EngineApp->recast->DeleteGO(selected);
						}

					}

					if (selected->isNavigable) {

						if (ImGui::BeginPopup("Change Navigation Area")) {
							std::string popupAreaName = App->detour->areaNames[popupArea];
							ImGui::Text(std::string("Do you want to change the navigation area to " + popupAreaName + " for all the child objects as well?").c_str());
							if (ImGui::Button("Yes, change children")) {
								std::queue<GameObject*> childs;
								childs.push(selected);

								while (!childs.empty()) {
									GameObject* current_child = childs.front();
									childs.pop();

									//We add all of its childs
									for (uint i = 0; i < current_child->childs.size(); i++)
										childs.push(current_child->childs[i]);

									//We change the value of static
									if (current_child->GetComponent<C_Mesh>() != nullptr)
										current_child->navigationArea = popupArea;
								}
								ImGui::CloseCurrentPopup();
							}
							ImGui::SameLine();
							if (ImGui::Button("No, this object only")) {
								selected->navigationArea = popupArea;
								ImGui::CloseCurrentPopup();
							}
							ImGui::SameLine();
							if (ImGui::Button("Cancel"))
								ImGui::CloseCurrentPopup();

							ImGui::EndPopup();
						}

						ImGui::Text("Navigation Area"); ImGui::SameLine();
						if (ImGui::BeginCombo("##areaCombo", App->detour->areaNames[selected->navigationArea])) {
							for (int i = 0; i < BE_DETOUR_TOTAL_AREAS; ++i) {
								std::string areaName = App->detour->areaNames[i];
								if (areaName != "") {
									ImGui::PushID((void*)App->detour->areaNames[i]);
									if (ImGui::Selectable(App->detour->areaNames[i], i == selected->navigationArea)) {
										if (selected->childs.size() != 0) {
											popupArea = i;
											openPopup = true;
										}
										else
											selected->navigationArea = i;
									}
									ImGui::PopID();
								}
							}
							ImGui::EndCombo();
						}
						if (openPopup) {
							ImGui::OpenPopup("Change Navigation Area");
							openPopup = false;
						}
					}

				}
				else
					ImGui::Text("Select one mesh from the scene.");
			}
			else {
				ImGui::Text("You selected multiple GOs!");
				ImGui::Text("Select just one mesh from the scene.");
			}

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