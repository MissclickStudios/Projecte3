#include "GameObject.h"

#include "C_Canvas.h"
#include "R_Shader.h"
#include "C_UI_Image.h"
#include "C_UI_Text.h"
#include "C_UI_Button.h"
#include "Player.h"
#include "HUBArmorer.h"

#include "DialogManager.h"

#include "Random.h"
#include "FileSystemDefinitions.h"
#include "Log.h"
#include "MC_Time.h"
#include "JSONParser.h"
#include "CoreDllHelpers.h"

#include "Application.h"
#include "M_Scene.h"
#include "M_FileSystem.h"
#include "M_Input.h"
#include "M_UISystem.h"
#include "DialogManager.h"

DialogManager::DialogManager()
{

}

DialogManager::~DialogManager()
{

}

void DialogManager::Start()
{
	GameObject* tmp = nullptr;

	tmp = App->scene->GetGameObjectByName(dialogCanvasName.c_str());
	if (tmp != nullptr)
		dialogCanvas = tmp->GetComponent<C_Canvas>();

	tmp = App->scene->GetGameObjectByName(speakerImageName.c_str());
	if (tmp != nullptr)
		speakerImage = tmp->GetComponent<C_UI_Image>();

	tmp = App->scene->GetGameObjectByName(textBackgroundName.c_str());
	if (tmp != nullptr)
		textBackground = tmp->GetComponent<C_UI_Image>();

	tmp = App->scene->GetGameObjectByName(speakerTextName.c_str());
	if (tmp != nullptr)
		speakerText = tmp->GetComponent<C_UI_Text>();

	tmp = App->scene->GetGameObjectByName(dialogTextName.c_str());
	if (tmp != nullptr)
		dialogText = tmp->GetComponent<C_UI_Text>();

	tmp = App->scene->GetGameObjectByName(dialogButtonName.c_str());
	if (tmp != nullptr)
		dialogButton = tmp->GetComponent<C_UI_Button>();

	tmp = App->scene->GetGameObjectByName(mandoName.c_str());
	if (tmp != nullptr)
		mando = (Player*)tmp->GetScript("Player");

	tmp = App->scene->GetGameObjectByName("HUBArmorer");
	if (tmp != nullptr)
		armorer = (HUBArmorer*)tmp->GetScript("HUBArmorer");
}

void DialogManager::Update()
{

	switch (state)
	{
		case DialogState::NO_DIALOG:

			break;
		case DialogState::SLIDE_IN:

			App->uiSystem->PushCanvas(dialogCanvas);
			StartTalking();

			break; 
		case DialogState::TALKING:
			//Update text until it's full

			if (dialogButton->GetState() == UIButtonState::PRESSEDIN)
			{
				dialogText->SetText(currentLine->lineText.c_str());
				state = DialogState::TALKED;
				currentLineLetter = 0;
			}

			nextLetterTimer += MC_Time::Game::GetDT();

			if (nextLetterTimer > textSpeed)
			{
				std::string tmp = dialogText->GetText();
				tmp.push_back(currentLine->lineText[currentLineLetter]);
				dialogText->SetText(tmp.c_str());
				currentLineLetter++;
				nextLetterTimer = 0;
				
				if (currentLineLetter >= currentLine->lineText.size())
				{
					state = DialogState::TALKED;
					currentLineLetter = 0;
				}
				/*else if (currentLine->lineText.at(currentLineLetter) == *" ")
				{
					dialogText->nextWordLetters = 0;
					wordIt = 1;
					while (currentLine->lineText.at(currentLineLetter + wordIt) != *" ")
					{
						if (currentLineLetter + wordIt >= currentLine->lineText.size() - 2)
							break;						
						wordIt++;
						dialogText->nextWordLetters++;
					}
				}*/
					
			}

			if (App->input->GetGameControllerButton(1) == ButtonState::BUTTON_DOWN || App->input->GetKey(SDL_SCANCODE_BACKSPACE) == KeyState::KEY_DOWN)
			{
				EndDialog();
			}

				break;
		case DialogState::TALKED:
			//Wait for input, then go to next phrase if there is any. Close dialog if not

			if (App->input->GetGameControllerButton(1) == ButtonState::BUTTON_DOWN || App->input->GetKey(SDL_SCANCODE_BACKSPACE) == KeyState::KEY_DOWN)
			{
				EndDialog();
			}

			if (dialogButton->GetState() == UIButtonState::PRESSEDIN)
			{
				//next line and go to talking or no dialog

					if (++currentLineIterator != currentDialog->lines.end()) //If it's not the last one
					{
						currentLine = (*currentLineIterator);
						StartNewLine();
						state = DialogState::TALKING;
					}
					else
					{
						EndDialog();
					}
			}

			break;
	}
}

void DialogManager::CleanUp()
{

}

void DialogManager::StartTalking()
{
	state = DialogState::TALKING;

	mando->SetPlayerInteraction(InteractionType::TALK);

	StartNewLine();
}

void DialogManager::StartNewLine()
{
	speakerText->SetText(currentLine->speakerName.c_str());

	//set speaker image
	
	if (strcmp(currentLine->speakerName.c_str(), "Mando") == 0)
	{
		//Set image to mando's portrait
		speakerImage->SetTextureCoordinates(-405, -1821, 320, 320);
	}
	if (strcmp(currentLine->speakerName.c_str(), "IG-11") == 0)
	{
		//Set image to mando's portrait
		speakerImage->SetTextureCoordinates(-1685, -1821, 320, 320);
	}
	if (strcmp(currentLine->speakerName.c_str(), "IG-12") == 0)
	{
		//Set image to mando's portrait
		speakerImage->SetTextureCoordinates(-1365, -1821, 320, 320);
	}
	if (strcmp(currentLine->speakerName.c_str(), "Grogu") == 0)
	{
		//Set image to mando's portrait
		speakerImage->SetTextureCoordinates(-725, -1821, 320, 320);
	}
	if (strcmp(currentLine->speakerName.c_str(), "Greef Karga") == 0)
	{
		//Set image to mando's portrait
		speakerImage->SetTextureCoordinates(-1045, -1821, 320, 320);
	}
	if (strcmp(currentLine->speakerName.c_str(), "Armorer") == 0)
	{
		//Set image to mando's portrait
		speakerImage->SetTextureCoordinates(-2005, -1821, 320, 320);
	}

	dialogText->SetText("");
	currentLineLetter = 0;
}

DialogSystem* DialogManager::LoadDialogSystem(const char* dialogName)
{
	std::string path = ASSETS_DIALOGS_PATH;
	path += dialogName;
	path += ".json";

	char* buffer = nullptr;
	App->fileSystem->Load(path.c_str(), &buffer);

	ParsonNode dialogRoot(buffer);

	CoreCrossDllHelpers::CoreReleaseBuffer(&buffer);

	//Fill Dialog System
	std::string dialogSystemName = dialogRoot.GetString("DialogSystemName");

	if (strcmp(dialogSystemName.c_str(), "[NOT FOUND]") == 0)
	{
		LOG("DialogSystem Lacks name");
		return nullptr;
	}
	
	DialogSystem* newDialogSystem = new DialogSystem(dialogSystemName.c_str());

	ParsonArray dialogPoolArray = dialogRoot.GetArray("DialogPool");

	for (uint i = 0; i < dialogPoolArray.size; i++)
	{
		ParsonNode dialogNode = dialogPoolArray.GetNode(i);

		std::string dialogName = dialogNode.GetString("DialogName");

		if (strcmp(dialogName.c_str(), "[NOT FOUND]") == 0)
		{
			LOG("DialogPool node lacks name");
			continue;
		}

		Dialog* newDialog = new Dialog(dialogName.c_str());

		ParsonArray dialogArray = dialogNode.GetArray("Lines");

		for (uint j = 0; j < dialogArray.size; j++)
		{
			ParsonNode phraseNode = dialogArray.GetNode(j);
			newDialog->lines.push_back(new DialogLine(phraseNode.GetString("Text"), phraseNode.GetString("Speaker")));
		}

		newDialogSystem->dialogPool.push_back(newDialog);
	}

	dialogSystemsLoaded.push_back(newDialogSystem);

	return newDialogSystem;
}

DialogState DialogManager::GetDialogState()
{
	return state;
}

void DialogManager::EndDialog()
{
	App->uiSystem->RemoveActiveCanvas(dialogCanvas);
	state = DialogState::NO_DIALOG;

	if (armorer != nullptr)
	{
		if (!armorer->menuOpen)
		{
			mando->SetPlayerInteraction(InteractionType::NONE);
		}
	}
	else
	{
		mando->SetPlayerInteraction(InteractionType::NONE);
	}
}

bool DialogManager::StartDialog(const char* dialogName)
{
	if (state != DialogState::NO_DIALOG)
		return false;

	bool found = false;

	//look for loaded dialogs
	for (auto loadedSystem = dialogSystemsLoaded.begin(); loadedSystem != dialogSystemsLoaded.end() ; ++loadedSystem)
	{
		if (strcmp((*loadedSystem)->dialogSystemName.c_str(), dialogName) == 0)
		{
			currentDialogSystem = (*loadedSystem);
			found = true;
		}
	}

	//if not found then load it
	if(!found)
		currentDialogSystem = LoadDialogSystem(dialogName);

	if (currentDialogSystem == nullptr)
	{
		LOG("Dialog %s could not be started. It wasn't found", dialogName);
		return false;
	}

	currentDialog = currentDialogSystem->dialogPool[Random::LCG::GetBoundedRandomUint(0, currentDialogSystem->dialogPool.size()-1)];
	currentLine = currentDialog->lines.front();
	currentLineIterator = currentDialog->lines.begin();

	state = DialogState::SLIDE_IN;

	return true;
}

void DialogManager::StartDialog(DialogSystem* dialogSystem)
{
	state = DialogState::SLIDE_IN;

	currentDialogSystem = dialogSystem;
}

DialogManager* CreateDialogManager()
{
	
	DialogManager* script = new DialogManager();
	INSPECTOR_STRING(script->dialogCanvasName);
	INSPECTOR_STRING(script->dialogTextName);
	INSPECTOR_STRING(script->dialogButtonName);
	INSPECTOR_STRING(script->speakerImageName);
	INSPECTOR_STRING(script->speakerTextName);
	INSPECTOR_STRING(script->textBackgroundName);
	return script;
	
}
