#include "Profiler.h"
#include "OpenGL.h"																	// ATTENTION: It was included AFTER ImGui.h
#include "ImGui.h"
#include "ImGui/include/imgui_internal.h"

#include "Icons.h"

#include "Application.h"															// ATTENTION: Globals.h already included in Module.h
#include "M_Window.h"
#include "M_Renderer3D.h"
#include "M_Camera3D.h"
#include "M_Input.h"
#include "M_Scene.h"
#include "M_FileSystem.h"
#include "M_ResourceManager.h"

#include "Importer.h"

#include "GameObject.h"

#include "EditorPanel.h"
#include "E_MainMenuBar.h"
#include "E_Toolbar.h"
#include "E_Configuration.h"
#include "E_Hierarchy.h"
#include "E_Inspector.h"
#include "E_Console.h"
#include "E_Project.h"
#include "E_Viewport.h"
#include "E_Resources.h"
#include "E_Timeline.h"
#include "E_ImGuiDemo.h"
#include "E_About.h"
#include "E_LoadFile.h"

#include "M_Editor.h"

#include "MemoryManager.h"

#pragma comment (lib, "Source/Dependencies/glew/libx86/glew32.lib")

M_Editor::M_Editor(bool isActive) : Module("Editor", isActive),
clearColor		(0.0f, 0.0f, 0.0f, 1.0f),
mainMenuBar	(new E_MainMenuBar()),
toolbar			(new E_Toolbar()),
configuration	(new E_Configuration()),
hierarchy		(new E_Hierarchy()),
inspector		(new E_Inspector()),
console			(new E_Console()),
project			(new E_Project()),
viewport		(new E_Viewport()),
resources		(new E_Resources()),
timeline		(new E_Timeline()),
imguiDemo		(new E_ImGuiDemo()),
about			(new E_About()),
loadFile		(new E_LoadFile())
{
	AddEditorPanel(mainMenuBar);
	AddEditorPanel(toolbar);
	AddEditorPanel(configuration);
	AddEditorPanel(hierarchy);
	AddEditorPanel(resources);
	AddEditorPanel(inspector);
	AddEditorPanel(timeline);
	AddEditorPanel(console);
	AddEditorPanel(project);
	AddEditorPanel(viewport);
	AddEditorPanel(imguiDemo);
	AddEditorPanel(about);
	AddEditorPanel(loadFile);

	showConfiguration	= true;
	showHierarchy		= true;
	showInspector		= true;
	showConsole			= true;
	showImguiDemo		= false;
	showAboutPopup		= false;
	showCloseAppPopup	= false;
	showLoadFilePopup	= false;
}

M_Editor::~M_Editor()
{
	for (uint i = 0; i < editorPanels.size(); ++i)
	{
		editorPanels[i]->CleanUp();
		RELEASE(editorPanels[i]);
	}

	editorPanels.clear();
}

bool M_Editor::Init(ParsonNode& config)
{
	bool ret = true;

	return ret;
}

bool M_Editor::Start()
{
	bool ret = true;

	InitializeImGui();
	
	return ret;
}

UpdateStatus M_Editor::PreUpdate(float dt)
{
	UpdateStatus ret = UpdateStatus::CONTINUE;

	EditorShortcuts();
	CheckShowHideFlags();

	return ret;
}

UpdateStatus M_Editor::Update(float dt)
{
	UpdateStatus ret = UpdateStatus::CONTINUE;

	return ret;
}

UpdateStatus M_Editor::PostUpdate(float dt)
{
	BROFILER_CATEGORY("Editor PostUpdate", Profiler::Color::IndianRed);
	
	UpdateStatus ret = UpdateStatus::CONTINUE;
	
	ImGuiIO& io = ImGui::GetIO();

	//Start Dear ImGui's frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(App->window->GetWindow());
	ImGui::NewFrame();

	if (BeginRootWindow(io, "Root window", true, ImGuiWindowFlags_MenuBar))
	{
		bool draw = true;
		for (uint i = 0; i < editorPanels.size(); ++i)
		{
			if (editorPanels[i]->IsActive())
			{
				draw = editorPanels[i]->Draw(io);

				if (!draw)
				{
					ret = UpdateStatus::STOP;
					LOG("[EDITOR] Exited through %s Panel", editorPanels[i]->GetName());
					break;
				}
			}
		}
		
		ImGui::End();
	}
	
	return ret;
}

bool M_Editor::CleanUp()
{	
	// ImGui CleanUp()
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	
	return true;
}

bool M_Editor::LoadConfiguration(ParsonNode& root)
{
	bool ret = true;

	return ret;
}

bool M_Editor::SaveConfiguration(ParsonNode& root) const
{
	bool ret = true;

	return ret;
}

// -------------- EDITOR METHODS --------------
bool M_Editor::GetEvent(SDL_Event* event) const
{
	return ImGui_ImplSDL2_ProcessEvent(event);											
}

void M_Editor::AddEditorPanel(EditorPanel* panel)
{
	editorPanels.push_back(panel);
}

void M_Editor::EditorShortcuts()
{
	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KeyState::KEY_DOWN)
	{
		if (showAboutPopup || showLoadFilePopup)
		{
			showAboutPopup		= false;
			showLoadFilePopup	= false;
		}
		else
		{
			showCloseAppPopup = !showCloseAppPopup;
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_LCTRL) == KeyState::KEY_REPEAT)
	{
		if (App->input->GetKey(SDL_SCANCODE_1) == KeyState::KEY_DOWN)
		{
			showConfiguration = !showConfiguration;
		}
		if (App->input->GetKey(SDL_SCANCODE_2) == KeyState::KEY_DOWN)
		{
			showHierarchy = !showHierarchy;
		}
		if (App->input->GetKey(SDL_SCANCODE_3) == KeyState::KEY_DOWN)
		{
			showInspector = !showInspector;
		}
		if (App->input->GetKey(SDL_SCANCODE_4) == KeyState::KEY_DOWN)
		{
			showConsole = !showConsole;
		}
		if (App->input->GetKey(SDL_SCANCODE_8) == KeyState::KEY_DOWN)
		{
			showImguiDemo = !showImguiDemo;
		}
		if (App->input->GetKey(SDL_SCANCODE_9) == KeyState::KEY_DOWN)
		{
			showAboutPopup = !showAboutPopup;
		}
		

		if (App->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_DOWN)
		{
			App->scene->SaveScene();
		}
		if (App->input->GetKey(SDL_SCANCODE_O) == KeyState::KEY_DOWN)
		{
			showLoadFilePopup = true;
		}
	}
}

void M_Editor::CheckShowHideFlags()
{	
	showConfiguration	?	configuration->Enable()	: configuration->Disable();				// Engine Configuration
	showHierarchy		?	hierarchy->Enable()		: hierarchy->Disable();					// Hierarchy
	showInspector		?	inspector->Enable()		: inspector->Disable();					// Inspector
	showConsole			?	console->Enable()		: console->Disable();					// Console
	showProject			?	project->Enable()		: project->Disable();					// Project
	showImguiDemo		?	imguiDemo->Enable()		: imguiDemo->Disable();					// ImGui Demo
	showAboutPopup		?	about->Enable()			: about->Disable();						// About Popup
	showLoadFilePopup	?	loadFile->Enable()		: loadFile->Disable();					// Load File
}

bool M_Editor::EditorIsBeingHovered() const
{
	for (uint i = 0; i < editorPanels.size(); ++i)
	{
		if (editorPanels[i]->IsHovered())
		{
			return true;
		}
	}

	return false;
}

bool M_Editor::RenderEditorPanels() const
{
	// Rendering all ImGui elements
	ImGuiIO& io = ImGui::GetIO();

	ImGui::Render();
	//glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	//glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
	//glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Updating and rendering additional platform windows
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		SDL_Window* backupCurrentWindow		= SDL_GL_GetCurrentWindow();
		SDL_GLContext backupCurrentContext	= SDL_GL_GetCurrentContext();

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		SDL_GL_MakeCurrent(backupCurrentWindow, backupCurrentContext);
	}

	return true;
}

bool M_Editor::InitializeImGui() const
{
	bool ret = true;

	// Setting up Dear ImGui's context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();																// Needs to be called multiple times during a frame to update IO correctly.
	(void)io;

	io.IniFilename = CONFIGURATION_PATH "imgui.ini";

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;										// Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;											// Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;											// Enable MultiViewport / Platform Windows

	ImGui::StyleColorsDark();																	// Setting up Dear ImGui's style.
	ImGuiStyle& style = ImGui::GetStyle();														// Tweaking the platform windows to look identical to regular ones.

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)										// Setting some configuration parameters in case the ViewportsEnable flag is enabled. 
	{																							// 
		style.WindowRounding = 0.0f;															// Setting the windows to have no rounding on their corners.
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;												// Setting the windows Alpha to 255, making them completely opaque.
	}																							// -----------------------------------------------

	ImGui_ImplSDL2_InitForOpenGL(App->window->GetWindow(), App->renderer->context);				// Setting up Platform/Renderer bindings
	ImGui_ImplOpenGL3_Init(0);																	// -------------------------------------

	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

	return ret;
}

bool M_Editor::GetShowWorldGrid() const
{
	return App->renderer->GetRenderWorldGrid();
}

bool M_Editor::GetShowWorldAxis() const
{
	return App->renderer->GetRenderWorldAxis();
}

bool M_Editor::GetShowPrimitiveExamples() const
{
	return App->renderer->GetRenderPrimitiveExamples();
}

void M_Editor::SetShowWorldGrid(bool setTo)
{
	App->renderer->SetRenderWorldGrid(setTo);
}

void M_Editor::SetShowWorldAxis(bool setTo)
{
	App->renderer->SetRenderWorldAxis(setTo);
}

void M_Editor::SetShowPrimitiveExamples(bool setTo)
{
	App->renderer->SetRenderPrimtiveExamples(setTo);
}

void M_Editor::UpdateFrameData(int frames, int ms)
{
	configuration->UpdateFrameData(frames, ms);
}

void M_Editor::AddConsoleLog(const char* log)
{
	if (editorPanels.size() > 0)
	{
		if (console != nullptr)
		{
			console->AddLog(log);
		}
	}
}

void M_Editor::AddInputLog(uint key, uint state)
{
	char input[128];
	const char* states[] = { "IDLE", "DOWN", "REPEAT", "UP" };										// We add the 4 main key states. In practice "IDLE" will not be displayed.
	
	if (configuration != nullptr)
	{
		if (key < App->input->GetMaxNumScancodes())
		{
			const char* keyName = SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)key));		// Through the scancode it is possible to get a string with the name of the key.

			sprintf_s(input, 128, "[KEY] %02u %s - %s\n", key, keyName, states[state]);
		}
		else
		{
			uint mouseButton = key - App->input->GetMaxNumScancodes();

			sprintf_s(input, 128, "[MOUSE] %02u - %s\n", mouseButton, states[state]);
		}

		configuration->AddInputLog(input);
	}
}

GameObject* M_Editor::GetSceneRootThroughEditor() const
{
	return App->scene->GetSceneRoot();
}

void M_Editor::SetSceneRootThroughEditor(GameObject* gameObject)
{
	App->scene->SetSceneRoot(gameObject);
}

GameObject* M_Editor::GetSelectedGameObjectThroughEditor() const
{
	return App->scene->GetSelectedGameObject();
}

void M_Editor::SetSelectedGameObjectThroughEditor(GameObject* gameObject)
{
	App->scene->SetSelectedGameObject(gameObject);
}

void M_Editor::DeleteSelectedGameObject()
{
	App->scene->DeleteSelectedGameObject();
}

void M_Editor::CreateGameObject(const char* name, GameObject* parent)
{
	App->scene->CreateGameObject(name, parent);
}

C_Camera* M_Editor::GetCurrentCameraThroughEditor() const
{
	return App->camera->GetCurrentCamera();
}

void M_Editor::SetCurrentCameraThroughEditor(C_Camera* gameObject)
{
	App->camera->SetCurrentCamera(gameObject);
}

void M_Editor::SetMasterCameraThroughEditor()
{
	App->camera->SetMasterCameraAsCurrentCamera();
}

float2 M_Editor::GetWorldMousePositionThroughEditor() const
{
	return this->viewport->GetWorldMousePosition();
}

float2 M_Editor::GetWorldMouseMotionThroughEditor() const
{
	return this->viewport->GetWorldMouseMotion();
}

float2 M_Editor::GetSceneTextureSizeThroughEditor() const
{
	return this->viewport->GetSceneTextureSize();
}

bool M_Editor::ViewportIsHovered() const
{
	return this->viewport->IsHovered();
}

bool M_Editor::UsingGuizmoInScene() const
{
	return this->viewport->UsingGuizmo();
}

bool M_Editor::HoveringGuizmo() const
{
	return this->viewport->HoveringGuizmo();
}

bool M_Editor::SelectedIsSceneRoot() const
{
	return (App->scene->GetSelectedGameObject() == App->scene->GetSceneRoot());
}

bool M_Editor::SelectedIsAnimationBone() const
{
	return App->scene->GetSelectedGameObject()->is_bone;
}

void M_Editor::GetEngineIconsThroughEditor(Icons& engineIcons)
{
	engineIcons = App->renderer->GetEngineIcons();
}

void M_Editor::LoadResourceIntoSceneThroughEditor()
{
	App->scene->LoadResourceIntoScene(project->GetDraggedResource());
}

void M_Editor::GetResourcesThroughEditor(std::map<uint32, Resource*>& resources) const
{
	//resources = App->resourceManager->GetResources();
	App->resourceManager->GetResources(resources);
}

void M_Editor::SaveSceneThroughEditor(const char* sceneName)
{
	App->scene->SaveScene(sceneName);
}

void M_Editor::LoadFileThroughEditor(const char* path)
{
	std::string extension = App->fileSystem->GetFileExtension(path);

	if (extension == "json" || extension == "JSON")
	{
		App->scene->LoadScene(path);
	}
	else
	{
		App->resourceManager->ImportFile(path);
	}
}

bool M_Editor::BeginRootWindow(ImGuiIO& io, const char* windowId, bool docking, ImGuiWindowFlags windowFlags)
{
	bool ret = true;
	
	ImGuiViewport* viewport = ImGui::GetWindowViewport();

	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus
					| ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ret = ImGui::Begin(windowId, &ret, windowFlags);
	ImGui::PopStyleVar(3);

	if (docking)
	{	
		BeginDockspace(io, windowId, ImGuiDockNodeFlags_PassthruCentralNode);
	}

	return ret;
}	
	
void M_Editor::BeginDockspace(ImGuiIO& io, const char* dockspaceId, ImGuiDockNodeFlags dockingFlags, ImVec2 size)
{
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dckspace_id = ImGui::GetID(dockspaceId);
		ImGui::DockSpace(dckspace_id, size, dockingFlags);
	}
}