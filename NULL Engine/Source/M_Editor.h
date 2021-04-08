#ifndef __M_EDITOR_H__
#define __M_EDITOR_H__

#include <map>

#include "ImGui/include/imgui.h"
#include "MathGeoLib/include/Math/float2.h"
#include "Module.h"
#include "VariableTypedefs.h"

struct Icons;
class ParsonNode;

class Resource;

class GameObject;
class C_Camera;

class EditorPanel;
class E_MainMenuBar;
class E_Toolbar;
class E_Configuration;
class E_Hierarchy;
class E_Inspector;
class E_Console;
class E_Project;
class E_Viewport;
class E_Resources;
class E_Timeline;
class E_Navigation;
class E_ImGuiDemo;
class E_About;
class E_LoadFile;
class E_SaveFile;
class E_WantToSaveScene;

enum class WantToSaveType;

typedef unsigned __int32 uint32;

struct ImGuiPayload;

enum class ImguiStyle
{
	CLASSIC,
	LIGHT,
	DARK,
	OTHER
};

enum class EditorEvent
{
	CREATE_GAMEOBJECT,
	SELECT_GAMEOBJECT,
	DELETE_GAME_OBJECT,
	LOAD_FILE
};

class M_Editor : public Module
{
public:
	M_Editor(bool isActive = true);
	~M_Editor();

	bool			Init				(ParsonNode& config) override;
	bool			Start				() override;
	UpdateStatus	PreUpdate			(float dt) override;
	UpdateStatus	Update				(float dt) override;
	UpdateStatus	PostUpdate			(float dt) override;
	bool			CleanUp				() override;

	bool			LoadConfiguration	(ParsonNode& root) override;
	bool			SaveConfiguration	(ParsonNode& root) const override;

public:
	void			AddEditorPanel						(EditorPanel* panel);					// Will add the E_Panel* passed as argument to the gui_panels vector.

	void			EditorShortcuts						();										// All the shortcuts related to the editor have been gathered here.
	void			CheckShowHideFlags					();										// Will check whether or not each of the panels must be enabled or disabled.

	bool			EditorIsBeingHovered				() const;								// Will check whether or not any of the editor panels is being hovered.
	bool			InitializeImGui						() const;								// Creates an ImGui Context and sets an initial configuration for it.
	
public:
	//bool			GetIOWantTextInput					() const;

public:																							// --- Panel/Window Methods. Acts as an interface between other modules and the panels.
	bool			GetShowWorldGrid					() const;								// Will return true if the World Grid is being currently shown.
	bool			GetShowWorldAxis					() const;								// Will return true if the World Axis is being currently shown.
	bool			GetShowPrimitiveExamples			() const;								// Will return true if the Primitive Examples are being currently shown.
	void			SetShowWorldGrid					(bool setTo);							// Will Enable or Disable the World Grid depending on the passed argument.
	void			SetShowWorldAxis					(bool setTo);							// Will Enable or Disable the depending on the passed argument.
	void			SetShowPrimitiveExamples			(bool setTo);							// Will Enable or Disable the depending on the passed argument.

	void			UpdateFrameData						(int frames, int ms);					// Configuration: Passing the received frame data to the configuration editor module.
	void			AddInputLog							(uint key, uint state);					// Configuration: Receives an input key and a state and passes a log to the config editor mod.

	void			AddConsoleLog						(const char* log) override;				// Console: Passing the received console log to the console editor module.

	GameObject*		GetSceneRootThroughEditor			() const;								// Hierarchy & inspector: Will return the current root GameObjcect.
	void			SetSceneRootThroughEditor			(GameObject* gameObject);				// Hierarchy & inspector: Will set the scene's root GameObject with the passed one.
	GameObject*		GetSelectedGameObjectThroughEditor	() const;								// Hierarchy & Inspector: Will return the currently selected GameObject.
	void			SetSelectedGameObjectThroughEditor	(GameObject* gameObject);				// Hierarchy & Inspector: Will set the scene's selected GameObject with the passed one.
	void			DeleteSelectedGameObject			();										// Hierarchy & Inspector: Will call the Module Scene's delete selected GameObject method.
	bool			SelectedIsSceneRoot					() const;								// Hierarchy & Inspector: Will check whether or not the selected GameObject is the scene root.
	bool			SelectedIsAnimationBone				() const;								// Hierarchy & Inspector: Will check whether or not the selected GameObject is an anim. bone.
	void			CreateGameObject					(const char* name, GameObject* parent);	// Hierarchy & Inspector: Will call the Module Scene's create GameObject method.
	C_Camera*		GetCurrentCameraThroughEditor		() const;								// Hierarchy & Inspector: 
	void			SetCurrentCameraThroughEditor		(C_Camera* gameObject);					// Hierarchy & Inspector:  
	void			SetMasterCameraThroughEditor		();										// Hierarchy & Inspector: 

	float2			GetWorldMousePositionThroughEditor	() const;								// Viewport: 
	float2			GetWorldMouseMotionThroughEditor	() const;								// Viewport: 
	float2			GetSceneTextureSizeThroughEditor	() const;								// Viewport: 
	bool			ViewportIsHovered					() const;								// Viewport: 
	bool			UsingGuizmoInScene					() const;								// Viewport: 
	bool			HoveringGuizmo						() const;								// Viewport: 

	void			GetEngineIconsThroughEditor			(Icons& engineIcons);					// Project:
	void			LoadResourceIntoSceneThroughEditor	(const ImGuiPayload& payload);							// Project: As of now Drag&Drop Source is in Project and Target in Viewport. TODO FIX LATER

	const std::map<uint32, Resource*>* GetResourcesThroughEditor () const;	// Resources: 

	void			SaveSceneThroughEditor				(const char* sceneName);
	void			LoadFileThroughEditor				(const char* path);						// Load File: Will send the given path to the Importer.

private:
	bool BeginRootWindow(ImGuiIO& io, const char* windowId, bool docking, ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None);							// Generates a root window for docking.
	void BeginDockspace(ImGuiIO& io, const char* dockspaceId, ImGuiDockNodeFlags dockingFlags = ImGuiDockNodeFlags_None, ImVec2 size = { 0.0f, 0.0f });	// Generates a new dockspace.

private:																						
	void EditorCameraUpdate();																	//Editor camera Update
	void ProcessInput(SDL_Event& event) override;												//Process ImGui events from SDL
	void PostSceneRendering() override;															//Render ImGui

public:
	void OpenWantToSaveScenePopup(WantToSaveType type);

public:
	std::vector<EditorPanel*>	editorPanels;													// Will store all the editor modules. Will be iterated for drawing all the panels.

	E_MainMenuBar*				mainMenuBar;
	E_Toolbar*					toolbar;
	E_Configuration*			configuration;
	E_Hierarchy*				hierarchy;
	E_Inspector*				inspector;
	E_Console*					console;
	E_Project*					project;
	E_Viewport*					viewport;
	E_Resources*				resources;
	E_Timeline*					timeline;
	E_Navigation*				navigation;
	E_ImGuiDemo*				imguiDemo;
	E_About*					about;
	E_LoadFile*					loadFile;
	E_SaveFile*					saveFile;
	E_WantToSaveScene*			wantToSaveScene;

	ImVec4						clearColor;														// Will be used to set the clear color of the rendering environment.

	bool						showConfiguration;												// Enable/Disable the Configuration window.
	bool						showHierarchy;													// Enable/Disable the Hierarchy window.
	bool						showInspector;													// Enable/Disable the Inspector window.
	bool						showConsole;													// Enable/Disable the Console window.
	bool						showProject;													// Enable/Disable the Project window.
	bool						showNavigation;													// Enable/Disable the Navigation window.
	bool						showImguiDemo;													// Enable/Disable the ImGui Demo window.

	bool						showAboutPopup;													// Enable/Disable the About window popup.
	bool						showLoadFilePopup;												// Enable/Disable the Load File popup.
	bool						showSaveFilePopup;
	bool						showCloseAppPopup;												// Enable/Disable the Close App popup.
	bool						showWantToSaveScenePopup;
};

#endif // !__M_EDITOR_H__