#include "Profiler.h"
#include "OpenGL.h"																	// ATTENTION: It was included AFTER ImGui.h
#include "ImGui.h"
#include "ImGui/include/imgui_internal.h"

#include "MC_Time.h"
#include "Log.h"

#include "Icons.h"

#include "EngineApplication.h"															// ATTENTION: Globals.h already included in Module.h
#include "M_Window.h"
#include "M_Renderer3D.h"
#include "M_Camera3D.h"
#include "M_Input.h"
#include "M_Scene.h"
#include "M_FileSystem.h"
#include "M_ResourceManager.h"

#include "Importer.h"

#include "GameObject.h"
#include "C_Transform.h"

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
#include "E_Navigation.h"
#include "E_ImGuiDemo.h"
#include "E_About.h"
#include "E_LoadFile.h"
#include "E_SaveFile.h"
#include "E_WantToSaveScene.h"

#include "M_Editor.h"

#include "MemoryManager.h"

#pragma comment (lib, "Source/Dependencies/glew/libx86/glew32.lib") //Needed for ImGui
#pragma comment (lib, "opengl32.lib")								//link Microsoft OpenGL lib


M_Editor::M_Editor(bool isActive) : Module("Editor", isActive),
clearColor		(0.0f, 0.0f, 0.0f, 1.0f),
mainMenuBar		(new E_MainMenuBar()),
toolbar			(new E_Toolbar()),
configuration	(new E_Configuration()),
hierarchy		(new E_Hierarchy()),
inspector		(new E_Inspector()),
console			(new E_Console()),
project			(new E_Project()),
viewport		(new E_Viewport()),
resources		(new E_Resources()),
timeline		(new E_Timeline()),
navigation		(new E_Navigation()),
imguiDemo		(new E_ImGuiDemo()),
about			(new E_About()),
loadFile		(new E_LoadFile()),
saveFile		(new E_SaveFile()),
wantToSaveScene (new E_WantToSaveScene())
{
	//16 = num of editor panels to pushback
	//if you create or remove 1 editor panel change the 16 accordingly
	editorPanels.reserve(16);

	AddEditorPanel(mainMenuBar);
	AddEditorPanel(toolbar);
	AddEditorPanel(configuration);
	AddEditorPanel(hierarchy);
	AddEditorPanel(resources);
	AddEditorPanel(timeline);
	AddEditorPanel(navigation);
	AddEditorPanel(inspector);
	AddEditorPanel(console);
	AddEditorPanel(project);
	AddEditorPanel(viewport);
	AddEditorPanel(imguiDemo);
	AddEditorPanel(about);
	AddEditorPanel(loadFile);
	AddEditorPanel(saveFile);
	AddEditorPanel(wantToSaveScene);

	showConfiguration			= true;
	showHierarchy				= true;
	showInspector				= true;
	showConsole					= true;
	showProject					= true;
	showNavigation				= true;
	showImguiDemo				= false;
	showAboutPopup				= false;
	showCloseAppPopup			= false;
	showLoadFilePopup			= false;
	showSaveFilePopup			= false;
	showWantToSaveScenePopup	= false;
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
	EditorShortcuts();
	CheckShowHideFlags();

	EditorCameraUpdate();

	return UpdateStatus::CONTINUE;
}

UpdateStatus M_Editor::Update(float dt)
{
	return UpdateStatus::CONTINUE;
}

UpdateStatus M_Editor::PostUpdate(float dt)
{
	OPTICK_CATEGORY("Editor Post Update", Optick::Category::Update);

	UpdateStatus ret = UpdateStatus::CONTINUE;
	
	ImGuiIO& io = ImGui::GetIO();

	//Start Dear ImGui's frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(EngineApp->window->GetWindow());
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
	
	// Editor: Configuration Frame Data Histograms
	UpdateFrameData(MC_Time::Real::GetFramesLastSecond(), MC_Time::Real::GetMsLastFrame());
	
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
void M_Editor::AddEditorPanel(EditorPanel* panel)
{
	editorPanels.push_back(panel);
}

void M_Editor::EditorShortcuts()
{
	if (EngineApp->input->GetKey(SDL_SCANCODE_ESCAPE) == KeyState::KEY_DOWN)
	{
		if (showAboutPopup || showLoadFilePopup || showSaveFilePopup || showWantToSaveScenePopup)
		{
			showAboutPopup				= false;
			showLoadFilePopup			= false;
			showSaveFilePopup			= false;
			showWantToSaveScenePopup	= false;
		}
		else
		{
			showCloseAppPopup = !showCloseAppPopup;
		}
	}

	if (EngineApp->input->GetKey(SDL_SCANCODE_LCTRL) == KeyState::KEY_REPEAT && EngineApp->gameState != GameState::PLAY)
	{
		if (EngineApp->input->GetKey(SDL_SCANCODE_1) == KeyState::KEY_DOWN)
		{
			showConfiguration = !showConfiguration;
		}
		if (EngineApp->input->GetKey(SDL_SCANCODE_2) == KeyState::KEY_DOWN)
		{
			showHierarchy = !showHierarchy;
		}
		if (EngineApp->input->GetKey(SDL_SCANCODE_3) == KeyState::KEY_DOWN)
		{
			showInspector = !showInspector;
		}
		if (EngineApp->input->GetKey(SDL_SCANCODE_4) == KeyState::KEY_DOWN)
		{
			showConsole = !showConsole;
		}
		if (EngineApp->input->GetKey(SDL_SCANCODE_8) == KeyState::KEY_DOWN)
		{
			showImguiDemo = !showImguiDemo;
		}
		if (EngineApp->input->GetKey(SDL_SCANCODE_9) == KeyState::KEY_DOWN)
		{
			showAboutPopup = !showAboutPopup;
		}
		

		if (EngineApp->input->GetKey(SDL_SCANCODE_S) == KeyState::KEY_DOWN)
		{
			showSaveFilePopup = true;
		}
		if (EngineApp->input->GetKey(SDL_SCANCODE_O) == KeyState::KEY_DOWN)
		{
			showLoadFilePopup = true;
		}
	}
}

void M_Editor::CheckShowHideFlags()
{	
	(showConfiguration)			?	configuration->Enable()		: configuration->Disable();					// Engine Configuration
	(showHierarchy)				?	hierarchy->Enable()			: hierarchy->Disable();						// Hierarchy
	(showInspector)				?	inspector->Enable()			: inspector->Disable();						// Inspector
	(showConsole)				?	console->Enable()			: console->Disable();						// Console
	(showProject)				?	project->Enable()			: project->Disable();						// Project
	(showNavigation)			?	navigation->Enable()		: navigation->Disable();					// Navigation
	(showImguiDemo)				?	imguiDemo->Enable()			: imguiDemo->Disable();						// ImGui Demo
	(showAboutPopup)			?	about->Enable()				: about->Disable();							// About Popup
	(showLoadFilePopup)			?	loadFile->Enable()			: loadFile->Disable();						// Load File
	(showSaveFilePopup)			?	saveFile->Enable()			: saveFile->Disable();						// Save File
	(showWantToSaveScenePopup)	?	wantToSaveScene->Enable()	: wantToSaveScene->Disable();				// Save Scene
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

	ImGui_ImplSDL2_InitForOpenGL(EngineApp->window->GetWindow(), EngineApp->renderer->context);	// Setting up Platform/Renderer bindings
	ImGui_ImplOpenGL3_Init(0);																	// -------------------------------------

	glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

	return ret;
}

bool M_Editor::GetShowWorldGrid() const
{
	return EngineApp->renderer->GetRenderWorldGrid();
}

bool M_Editor::GetShowWorldAxis() const
{
	return EngineApp->renderer->GetRenderWorldAxis();
}

bool M_Editor::GetShowPrimitiveExamples() const
{
	return EngineApp->renderer->GetRenderPrimitiveExamples();
}

void M_Editor::SetShowWorldGrid(bool setTo)
{
	EngineApp->renderer->SetRenderWorldGrid(setTo);
}

void M_Editor::SetShowWorldAxis(bool setTo)
{
	EngineApp->renderer->SetRenderWorldAxis(setTo);
}

void M_Editor::SetShowPrimitiveExamples(bool setTo)
{
	EngineApp->renderer->SetRenderPrimtiveExamples(setTo);
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
		if (key < EngineApp->input->GetMaxNumScancodes())
		{
			const char* keyName = SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)key));		// Through the scancode it is possible to get a string with the name of the key.

			sprintf_s(input, 128, "[KEY] %02u %s - %s\n", key, keyName, states[state]);
		}
		else
		{
			uint mouseButton = key - EngineApp->input->GetMaxNumScancodes();

			sprintf_s(input, 128, "[MOUSE] %02u - %s\n", mouseButton, states[state]);
		}

		configuration->AddInputLog(input);
	}
}

GameObject* M_Editor::GetSceneRootThroughEditor() const
{
	return EngineApp->scene->GetSceneRoot();
}

void M_Editor::SetSceneRootThroughEditor(GameObject* gameObject)
{
	EngineApp->scene->SetSceneRoot(gameObject);
}

GameObject* M_Editor::GetSelectedGameObjectThroughEditor() const
{
	return EngineApp->scene->GetSelectedGameObject();
}

void M_Editor::SetSelectedGameObjectThroughEditor(GameObject* gameObject)
{
	EngineApp->scene->SetSelectedGameObject(gameObject);
}

void M_Editor::DeleteSelectedGameObject()
{
	EngineApp->scene->DeleteSelectedGameObject();
}

void M_Editor::CreateGameObject(const char* name, GameObject* parent)
{
	EngineApp->scene->CreateGameObject(name, parent);
}

C_Camera* M_Editor::GetCurrentCameraThroughEditor() const
{
	return EngineApp->camera->GetCurrentCamera();
}

void M_Editor::SetCurrentCameraThroughEditor(C_Camera* gameObject)
{
	EngineApp->camera->SetCurrentCamera(gameObject);
}

void M_Editor::SetMasterCameraThroughEditor()
{
	EngineApp->camera->SetMasterCameraAsCurrentCamera();
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
	return (EngineApp->scene->GetSelectedGameObject() == EngineApp->scene->GetSceneRoot());
}

bool M_Editor::SelectedIsAnimationBone() const
{
	return EngineApp->scene->GetSelectedGameObject()->isBone;
}

void M_Editor::GetEngineIconsThroughEditor(Icons& engineIcons)
{
	engineIcons = EngineApp->renderer->GetEngineIcons();
}

void M_Editor::LoadResourceIntoSceneThroughEditor(const ImGuiPayload& payload)
{
	const char* draggedAssetPath = *(const char**)payload.Data;
	if (App->fileSystem->GetFileExtension(draggedAssetPath) == "prefab")
	{
		std::string prefabId;
		EngineApp->fileSystem->SplitFilePath(draggedAssetPath, nullptr, &prefabId, nullptr);
		EngineApp->resourceManager->LoadPrefab(std::stoi(prefabId), EngineApp->scene->GetSceneRoot());
	}
	else
	{
		Resource* draggedResource = EngineApp->resourceManager->GetResourceFromLibrary(draggedAssetPath);
		if (draggedResource != nullptr)
			EngineApp->scene->LoadResourceIntoScene(draggedResource);
	}
}

const std::map<uint32, Resource*>* M_Editor::GetResourcesThroughEditor() const
{
	return EngineApp->resourceManager->GetResourcesMap();
}

void M_Editor::SaveSceneThroughEditor(const char* sceneName)
{
	EngineApp->scene->SaveScene(sceneName);
}

void M_Editor::LoadFileThroughEditor(const char* path)
{
	//TODO: return string from dll memo leak
	//----------------call to get file expension function---------------------------
	std::string fullPath = path;
	std::string extension = "";

	size_t dotPosition = fullPath.find_last_of(".");

	extension = (dotPosition != fullPath.npos) ? fullPath.substr(dotPosition + 1) : "[NONE]";
	//--------------------------------------------------------------------------------------
	//std::string extension = EngineApp->fileSystem->GetFileExtension(path);
	//if i call it i get a memory leak when i return the std::string ???

	if (extension == "json" || extension == "JSON")
	{
		EngineApp->scene->LoadScene(path);
	}
	else
	{
		EngineApp->resourceManager->ImportFile(path);
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


//EDITOR CAMERA FUNCTIONALLITY
void M_Editor::EditorCameraUpdate() {

	if (ViewportIsHovered())
	{
		if (!HoveringGuizmo())
		{
			if (EngineApp->input->GetMouseButton(SDL_BUTTON_LEFT) == KeyState::KEY_DOWN)
			{
				EngineApp->camera->CastRay(GetWorldMousePositionThroughEditor());
			}
		}

		if (EngineApp->input->GetMouseButton(SDL_BUTTON_RIGHT) == KeyState::KEY_REPEAT)
		{
			EngineApp->camera->WASDMovement();

			EngineApp->camera->FreeLookAround();
		}

		if (EngineApp->input->GetKey(SDL_SCANCODE_LALT) == KeyState::KEY_REPEAT)
		{
			if (EngineApp->input->GetMouseButton(SDL_BUTTON_RIGHT) == KeyState::KEY_REPEAT)
			{
				if (EngineApp->scene->GetSelectedGameObject() != nullptr)
				{
					if (EngineApp->scene->GetSelectedGameObject()->GetComponent<C_Camera>() != EngineApp->camera->currentCamera)
					{
						EngineApp->camera->reference = EngineApp->scene->GetSelectedGameObject()->GetComponent<C_Transform>()->GetWorldPosition();
					}
				}
				else
				{
					EngineApp->camera->reference = float3::zero;
				}

				EngineApp->camera->Orbit(GetWorldMouseMotionThroughEditor());
			}
		}

		if (EngineApp->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KeyState::KEY_REPEAT)
		{
			EngineApp->camera->PanCamera(GetWorldMouseMotionThroughEditor());
		}

		if (EngineApp->input->GetMouseZ() != 0)
		{
			EngineApp->camera->Zoom();
		}

		if (EngineApp->input->GetKey(SDL_SCANCODE_LCTRL) == KeyState::KEY_IDLE)
		{
			if (EngineApp->input->GetKey(SDL_SCANCODE_O) == KeyState::KEY_DOWN)
			{
				EngineApp->camera->ReturnToWorldOrigin();
			}
		}

		if (EngineApp->scene->GetSelectedGameObject() != nullptr)
		{
			if (EngineApp->input->GetKey(SDL_SCANCODE_F) == KeyState::KEY_DOWN)
			{
				float3 target = EngineApp->scene->GetSelectedGameObject()->GetComponent<C_Transform>()->GetWorldPosition();
				EngineApp->camera->Focus(target);
			}
		}
	}
}

void M_Editor::ProcessInput(SDL_Event& event)
{
	ImGui_ImplSDL2_ProcessEvent(&event);
}

void M_Editor::PostSceneRendering()
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
		SDL_Window* backupCurrentWindow = SDL_GL_GetCurrentWindow();
		SDL_GLContext backupCurrentContext = SDL_GL_GetCurrentContext();

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		SDL_GL_MakeCurrent(backupCurrentWindow, backupCurrentContext);
	}
}

void M_Editor::OpenWantToSaveScenePopup(WantToSaveType type)
{
	showWantToSaveScenePopup = true;

	wantToSaveScene->type = type;
}
