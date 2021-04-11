#include "Profiler.h"

#include "Log.h"

#include "Color.h"

#include "EngineApplication.h"
#include "M_Window.h"
#include "M_Editor.h"

#include "E_Console.h"

#include "MemoryManager.h"

#define MAX_CONSOLE_LOG_SIZE	1000
#define MAX_SEARCH_STR_SIZE		128	

E_Console::E_Console() : EditorPanel("Console"),
scrollToBottom		(true),
showConsole			(IsActive()),
printErrorLogs		(true),
printWarningLogs	(true),
printStatusLogs		(true),
printFileSystemLogs	(true),
printImporterLogs	(true),
printSceneLogs		(true),
printNonTaggedLogs	(true)
{
	searchStr		= new char[MAX_SEARCH_STR_SIZE];
	searchStr[0]	= '\0';
}

E_Console::~E_Console()
{
	RELEASE_ARRAY(searchStr);
}

bool E_Console::Draw(ImGuiIO& io)
{
	OPTICK_CATEGORY("E_Console Draw",Optick::Category::Editor)
	bool ret = true;


	ImGui::Begin(GetName(), NULL, ImGuiWindowFlags_MenuBar);
	
	SetIsHovered();

	ConsoleMenuBar();													// Constructs the menu bar of the Console panel. It has an Options menu that allows to Clear or Close the console.
	ConsoleOutput();													// Prints all the logs in the console and formats them according to their type ([ERROR], [WARNING]...)

	ImGui::End();

	return ret;
}

bool E_Console::CleanUp()
{
	ClearLog();

	return true;
}

void E_Console::AddLog(const char* log)
{	
	if (logs.size() + 1 > MAX_CONSOLE_LOG_SIZE)
	{
		ClearLog();
		//LOG("[STATUS] Console Editor Panel: Cleared Console Logs! Event: Exceeded maximum console log size");
	}
	
	//--------------------------------------------------------------------TODO:Console memo leak solved--------------------------------------------------------------------------------
	//char* tmp = _strdup(log);											// strdup() duplicates the log string. This is necessary so when log is modified the console strings remain the same.
	char* tmp = (char*)malloc(strlen(log)+1);							//allocating memory for the string outside the dll boundary (+1 nullterminated character)
	strcpy(tmp, log);													//duplicating the string from the dll to the .exe
	logs.push_back(tmp);

	scrollToBottom = true;
}

void E_Console::ClearLog()
{
	for (uint i = 0; i < logs.size(); ++i)										// Iterates through all the strings inside logs and frees them from memory.
	{
		free(logs[i]);
	}

	logs.clear();

	scrollToBottom = true;
}

void E_Console::ConsoleMenuBar()
{
	ImGui::BeginMenuBar();

	if (ImGui::BeginMenu("Options"))
	{
		if (ImGui::BeginMenu("Log Filters"))
		{
			ImGui::MenuItem("Error", nullptr, &printErrorLogs);
			ImGui::MenuItem("Warning", nullptr, &printWarningLogs);
			ImGui::MenuItem("Status", nullptr, &printStatusLogs);
			ImGui::MenuItem("File System", nullptr, &printFileSystemLogs);
			ImGui::MenuItem("Importer", nullptr, &printImporterLogs);
			ImGui::MenuItem("Scene", nullptr, &printSceneLogs);
			ImGui::MenuItem("Nontagged", nullptr, &printNonTaggedLogs);

			ImGui::EndMenu();
		}

		if (ImGui::MenuItem("Clear Console"))	{ ClearLog(); }
		if (ImGui::MenuItem("Close Console"))	{ Disable(); }

		ImGui::EndMenu();
	}

	ImGui::EndMenuBar();
}

void E_Console::ConsoleOutput()
{
	ImGui::InputTextEx("Search", nullptr, searchStr, MAX_SEARCH_STR_SIZE, ImVec2(300.0f, 0.0f), ImGuiInputTextFlags_EnterReturnsTrue);
	
	ImGui::BeginChild("Console Output", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 6));

	ImVec4 textColour = White.C_Array();
	for (uint i = 0; i < logs.size(); ++i)
	{
		if (strstr(logs[i], searchStr) == nullptr)
		{
			continue;
		}
		
		if (strstr(logs[i], "[ERROR]") != nullptr)								// strstr() will look for a specific substring in the given string. Returns nullptr if the substr is not found.
		{
			if (!printErrorLogs)		{ continue; }							// If the Filter Tag is set not to be printed, then continue to the next iteration of the loop.
			textColour = LightRed.C_Array();										// [ERROR] logs will be Red.
		}
		else if (strstr(logs[i], "[WARNING]") != nullptr)
		{
			if (!printWarningLogs)		{ continue; }
			textColour = LightYellow.C_Array();									// [WARNING] logs will be Yellow.
		}
		else if (strstr(logs[i], "[STATUS]") != nullptr)
		{
			if (!printStatusLogs)		{ continue; }
			textColour = LightMagenta.C_Array();									// [STATUS] logs will be Magenta.
		}
		else if (strstr(logs[i], "[FILE_SYSTEM]") != nullptr)
		{
			if (!printFileSystemLogs)	{ continue; }
			textColour = LightBlue.C_Array();									// [FILE_SYSTEM] logs will be Blue.
		}
		else if (strstr(logs[i], "[IMPORTER]") != nullptr)
		{
			if (!printImporterLogs)		{ continue; }
			textColour = LightCyan.C_Array();									// [IMPORTER] logs will be Cyan.
		}
		else if (strstr(logs[i], "[SCENE]") != nullptr)
		{
			if (!printSceneLogs)		{ continue; }
			textColour = LightGreen.C_Array();									// [SCENE] logs will be Green.
		}
		else
		{
			if (!printNonTaggedLogs)	{ continue; }
			textColour = White.C_Array();										// Nontagged logs will be White.
		}

		ImGui::PushStyleColor(ImGuiCol_Text, textColour);
		ImGui::TextUnformatted(logs[i]);									// Raw text without formatting. Fast, without memory copies and no buffer size limits. For long chunks of txt.
		ImGui::PopStyleColor();
	}

	ImGui::PopStyleVar();
	ConsoleScrollToBottom();
	ImGui::EndChild();
}

void E_Console::ConsoleScrollToBottom()
{
	if (scrollToBottom)
	{
		ImGui::SetScrollHere(1.f);											// Sets the scroll position to the given one. From 0.0f to 1.0f (From above first item to below last item).

		scrollToBottom = false;
	}
}