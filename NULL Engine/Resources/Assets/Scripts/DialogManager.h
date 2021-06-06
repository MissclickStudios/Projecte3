#pragma once

#include "Script.h"
#include "ScriptMacros.h"
#include <map>

#include "MathGeoLib/include/Math/float3.h"

typedef unsigned int uint;
class R_Texture;
class C_Canvas;
class C_UI_Image;
class C_UI_Text;
class C_UI_Button;
class Player;
class HUBArmorer;

enum class DialogState
{
	SLIDE_IN,
	TALKING,
	TALKED,
	NO_DIALOG
};

struct DialogLine
{
	DialogLine(const char* _text, const char* _speakerName)
	{
		lineText = _text;
		speakerName = _speakerName;
	}

	std::string lineText = "This Phrase is empty";
	std::string speakerName = "Default Speaker";
};

struct Dialog
{
	Dialog(const char* name)
	{
		dialogName = name;
	}

	std::string dialogName = "Default Name";

	std::vector<DialogLine*> lines;
};

struct DialogSystem
{
	DialogSystem(const char* name)
	{
		dialogSystemName = name;
	}

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

	void StartTalking();
	void StartNewLine();

	DialogSystem* LoadDialogSystem(const char* dialogName); //Loads dialog for the scene to be used later (Pass only the name of the dialog)

	DialogState GetDialogState();

	void EndDialog();
private:

public:

	//StartDialog
	bool StartDialog(const char* dialogName); // dialogName = Name of the dialog only
	void StartDialog(DialogSystem* dialogSystem);

public:
	C_Canvas* dialogCanvas = nullptr;
	C_UI_Image* speakerImage = nullptr;
	C_UI_Image* textBackground = nullptr;
	C_UI_Text* speakerText = nullptr;
	C_UI_Text* dialogText = nullptr;
	C_UI_Button* dialogButton = nullptr;

	Player* mando = nullptr;

	HUBArmorer* armorer = nullptr;

	std::string dialogCanvasName = "DialogCanvas";
	std::string speakerImageName = "SpeakerImage";
	std::string textBackgroundName = "DialogBackground";
	std::string speakerTextName = "SpeakerText";
	std::string dialogTextName = "DialogText";
	std::string dialogButtonName = "DialogButton";
	std::string mandoName = "Mandalorian";

private:
	std::vector<DialogSystem*> dialogSystemsLoaded; //All the dialogs the scene has loaded in

	DialogState state = DialogState::NO_DIALOG;

	std::string currentText;

	DialogSystem* currentDialogSystem = nullptr;
	Dialog* currentDialog = nullptr;
	DialogLine* currentLine = nullptr;
	std::vector<DialogLine*>::iterator currentLineIterator;

	uint currentLineLetter = 0; //Holds the current letter position in string that has to be added in the line
	uint wordIt = 0;

	float textSpeed = 0.1f;
	float nextLetterTimer = 0.f;
};

SCRIPTS_FUNCTION DialogManager* CreateDialogManager();