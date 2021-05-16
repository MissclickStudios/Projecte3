#pragma once

#include "Script.h"
#include "ScriptMacros.h"
#include <map>

#include "MathGeoLib/include/Math/float3.h"

typedef unsigned int uint;
class R_Texture;
class C_UI_Image;
class C_UI_Text;

enum class DialogState
{
	SLIDE_IN,
	TALKING,
	TALKED,
	NO_DIALOG
};

struct DialogPhrase
{
	std::string text = "This Phrase is empty";

	std::string speakerName = "Default Speaker";
};

struct Dialog
{
	std::string dialogName = "Default Name";

	std::vector<DialogPhrase*> phrases;
};

struct DialogSystem
{
	std::string dialogSystemName = "Default Name";

	std::vector<Dialog*> dialogPool; //All the dialogs that can randomly trigger when Starting this dialog (There cana lso be just one)
};

class SCRIPTS_API DialogManager : public Script {
public:
	DialogManager();
	~DialogManager();

	void Start() override;
	void Update() override;
	void CleanUp()override;

	DialogSystem* LoadDialogSystem(const char* path); //Loads dialog for the scene to be used later (path starts from Assets/Dialogs/)

	//StartDialog
	void StartDialog(const char* dialogName);
	void StartDialog(DialogSystem* dialogSystem);

	std::vector<DialogSystem*> dialogSystemsLoaded; //All the dialogs the scene has loaded in

	DialogState state = DialogState::NO_DIALOG;

	C_UI_Image* speakerImage = nullptr;
	C_UI_Image* textBackground = nullptr;
	C_UI_Text* speakerText = nullptr;
	C_UI_Text* dialogText = nullptr;
};

SCRIPTS_FUNCTION DialogManager* CreateDialogManager();