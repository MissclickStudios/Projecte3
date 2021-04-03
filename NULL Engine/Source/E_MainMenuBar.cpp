#include "EngineApplication.h"
#include "M_Renderer3D.h"
#include "M_Editor.h"
#include "M_Scene.h"
#include "Profiler.h"
#include "E_MainMenuBar.h"

#include "E_WantToSaveScene.h"

#include "MemoryManager.h"

E_MainMenuBar::E_MainMenuBar() : EditorPanel("MainMenuBar")
{

}

E_MainMenuBar::~E_MainMenuBar()
{

}

bool E_MainMenuBar::Draw(ImGuiIO& io)
{
	bool ret = true;

	ImGui::BeginMainMenuBar();

	SetIsHovered();
	
	FileMainMenuItem();
	EditMainMenuItem();
	WindowMainMenuItem();
	ViewMainMenuItem();
	GameObjectsMainMenuItem();
	CreateMainMenuItem();
	HelpMainMenuItem();

	if (EngineApp->editor->showCloseAppPopup)
	{
		CloseAppPopup();																	// Not actually inside MainMenuBar but related to FileMainMenuItem().
	}

	ImGui::EndMainMenuBar();

	return ret;
}

bool E_MainMenuBar::CleanUp()
{
	bool ret = true;

	return ret;
}

// -------- TOOLBAR METHODS --------
bool E_MainMenuBar::FileMainMenuItem()
{
	bool ret = true;
	
	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New Scene", "Ctrl+N"))
		{
			//should ask if want to save current scene before going to new scene

			EngineApp->editor->OpenWantToSaveScenePopup(WantToSaveType::NEW_SCENE);
		}
		
		if (ImGui::MenuItem("Open Scene", "Ctrl+O"))
		{
			EngineApp->editor->showLoadFilePopup = true;
		}
		ImGui::Separator();

		if (ImGui::MenuItem("Save", "Ctrl+S"))
		{
			EngineApp->scene->SaveScene(App->scene->GetCurrentScene());
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Save As"))
		{
			EngineApp->editor->showSaveFilePopup = true;
			
			LOG("[SCENE] SAVED THE SCENE >:DDD");
		}



		
		
		ImGui::MenuItem("Save As...", "Ctrl+Shift+S", nullptr, false);

		ImGui::Separator();

		ImGui::MenuItem("New Project", "Ctrl+Shift+N", nullptr, false);
		ImGui::MenuItem("Open Project", "Ctrl+Shift+O", nullptr, false);
		ImGui::MenuItem("Save Project", "Ctrl+Alt+S", nullptr, false);

		ImGui::Separator();
		
		ImGui::MenuItem("Quit", "ESC", &EngineApp->editor->showCloseAppPopup);		// MenuItem(Item name string, shortcut string, bool to modify / get modified by)

		ImGui::EndMenu();
	}

	return ret;
}

bool E_MainMenuBar::EditMainMenuItem()
{
	bool ret = true;

	if (ImGui::BeginMenu("Edit"))
	{
		ImGui::MenuItem("Undo", "Ctrl+Z", nullptr, false);
		ImGui::MenuItem("Redo", "Ctrl+Y", nullptr, false);

		ImGui::Separator();

		ImGui::MenuItem("Select All", "Ctrl+A", nullptr, false);
		ImGui::MenuItem("Deselect All", "Shift+D", nullptr, false);
		ImGui::MenuItem("Select Children", "Shift+C", nullptr, false);
		ImGui::MenuItem("Invert Selection", "Ctrl+I", nullptr, false);

		ImGui::Separator();

		ImGui::MenuItem("Duplicate", "Ctrl+D", nullptr, false);
		ImGui::MenuItem("Delete", "Supr", nullptr, false);

		ImGui::Separator();

		ImGui::MenuItem("Play", "Ctrl+P", nullptr, false);
		ImGui::MenuItem("Pause", "Ctrl+Shift+P", nullptr, false);
		ImGui::MenuItem("Step", "Ctrl+Alt+P", nullptr, false);

		ImGui::Separator();

		ImGui::MenuItem("Settings");

		ImGui::EndMenu();
	}

	return ret;
}

bool E_MainMenuBar::WindowMainMenuItem()
{
	bool ret = true;

	if (ImGui::BeginMenu("Window"))
	{
		ImGui::MenuItem("Configuration", "1", &EngineApp->editor->showConfiguration);
		ImGui::MenuItem("Hierarchy", "2", &EngineApp->editor->showHierarchy);
		ImGui::MenuItem("Inspector", "3", &EngineApp->editor->showInspector);
		ImGui::MenuItem("Console", "4", &EngineApp->editor->showConsole);
		ImGui::MenuItem("GuiDemo", "8", &EngineApp->editor->showImguiDemo);
		ImGui::MenuItem("About", "9", &EngineApp->editor->showAboutPopup);

		ImGui::EndMenu();
	}

	return ret;
}

bool E_MainMenuBar::ViewMainMenuItem()
{
	bool ret = true;

	if (ImGui::BeginMenu("View"))
	{
		bool showGrid = EngineApp->editor->GetShowWorldGrid();
		ImGui::MenuItem("World Grid", "F1", &showGrid);
		EngineApp->editor->SetShowWorldGrid(showGrid);
		
		bool showAxis = EngineApp->editor->GetShowWorldAxis();
		ImGui::MenuItem("World Axis", "F2", &showAxis);
		EngineApp->editor->SetShowWorldAxis(showAxis);

		ImGui::EndMenu();
	}

	return ret;
}

bool E_MainMenuBar::GameObjectsMainMenuItem()
{
	bool ret = true;

	if (ImGui::BeginMenu("GameObject"))
	{
		if (ImGui::MenuItem("Create Empty"))
		{
			EngineApp->scene->CreateGameObject("EmptyGameObject", EngineApp->scene->GetSceneRoot());

			LOG("[SCENE] Created An Empty Object. Root Object defaults as parent.");
		}
		
		if (ImGui::MenuItem("Create Empty x10"))
		{
			for (uint i = 0; i < 10; ++i)
			{
				EngineApp->scene->CreateGameObject("EmptyGameObject", EngineApp->scene->GetSceneRoot());
			}

			LOG("[SCENE] Created 10 Empty Objects. Root Object defaults as parent.");
		}
		
		if (ImGui::BeginMenu("Primitives"))
		{
			bool showPrimitiveExamples = EngineApp->editor->GetShowPrimitiveExamples();
			ImGui::MenuItem("Show Examples", "", &showPrimitiveExamples);
			EngineApp->editor->SetShowPrimitiveExamples(showPrimitiveExamples);

			ImGui::Separator();

			ImGui::MenuItem("Cube", "", nullptr, false);
			ImGui::MenuItem("Sphere", "", nullptr, false);
			ImGui::MenuItem("Cylinder", "", nullptr, false);
			ImGui::MenuItem("Pyramid", "", nullptr, false);

			ImGui::EndMenu();
		}

		ImGui::MenuItem("Camera");

		ImGui::Separator();

		if (ImGui::BeginMenu("Draw Mode"))
		{
			bool mesh = !EngineApp->renderer->GetRenderWireframes();
			bool wire = EngineApp->renderer->GetRenderWireframes();
			bool vert = false;

			if (ImGui::MenuItem("Mesh", "TBD", &mesh))
			{
				if (wire)
				{
					EngineApp->renderer->SetRenderWireframes(false);
				}
			}
			
			if (ImGui::MenuItem("Wireframe", "F3", &wire))
			{
				EngineApp->renderer->SetRenderWireframes(wire);
			}
			
			ImGui::MenuItem("Vertices", "TBD", &vert, false);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Show", false))
		{
			bool norm = false;

			ImGui::MenuItem("Show Normals", "F4", &norm);

			ImGui::EndMenu();
		}

		ImGui::End();
	}

	return ret;
}

bool E_MainMenuBar::CreateMainMenuItem()
{

	if (ImGui::BeginMenu("Create"))
	{

		if (ImGui::MenuItem("Directional Light"))
		{
			App->renderer->GenerateSceneLight(Color(1.0f, 1.0f, 1.0f, 1.0f), Color(0.6, 0.6, 0.6, 0.5), Color(0.6, 0.6, 0.6, 0.5), LightType::DIRECTIONAL);
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Point Light"))
		{
			App->renderer->GenerateSceneLight(Color(1.0f, 1.0f, 1.0f, 1.0f), Color(0.6, 0.6, 0.6, 0.5), Color(0.6, 0.6, 0.6, 0.5), LightType::POINTLIGHT);
		}


		ImGui::EndMenu();
	}

	
	return false;
}

bool E_MainMenuBar::HelpMainMenuItem()
{
	bool ret = true;

	if (ImGui::BeginMenu("Help"))
	{
		if (ImGui::Button("About NULL Engine"))
		{
			EngineApp->editor->showAboutPopup = true;
		}
		
		ImGui::Separator();
		
		if (ImGui::MenuItem("Documentation"))
		{
			EngineApp->RequestBrowser("https://github.com/BarcinoLechiguino/NULL_Engine");
		}

		if (ImGui::MenuItem("Download Latest"))
		{
			EngineApp->RequestBrowser("https://github.com/BarcinoLechiguino/NULL_Engine/tags");
		}

		if (ImGui::MenuItem("Report a Bug"))
		{
			EngineApp->RequestBrowser("https://github.com/BarcinoLechiguino/NULL_Engine/issues/new");
		}

		ImGui::EndMenu();
	}

	return ret;
}

bool E_MainMenuBar::CloseAppPopup()
{
	bool ret = true;

	ImGui::OpenPopup("Close Application?");

	if (ImGui::BeginPopupModal("Close Application?"))
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 0.25f));
		if (ImGui::Button("CONFIRM"))
		{
			ImGui::CloseCurrentPopup();
			EngineApp->editor->showCloseAppPopup = false;

			EngineApp->quit = true;
		}
		ImGui::PopStyleColor();

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 0.25f));
		if (ImGui::Button("CANCEL"))
		{
			ImGui::CloseCurrentPopup();
			EngineApp->editor->showCloseAppPopup = false;
		}
		ImGui::PopStyleColor();

		ImGui::EndPopup();
	}

	return ret;
}