#include "Macros.h"

#include "Application.h"
#include "M_Window.h"
#include "M_Editor.h"

#include "E_Console.h"

#define MAX_CONSOLE_LOG_SIZE 1000

E_Console::E_Console() : EditorPanel("Console")
{
	scroll_to_bottom = true;
	show_console = IsActive();
}

E_Console::~E_Console()
{

}

bool E_Console::Draw(ImGuiIO& io)
{
	bool ret = true;

	ImGui::Begin(GetName(), NULL, ImGuiWindowFlags_MenuBar);
	
	SetIsHovered();

	ConsoleMenuBar();													// Constructs the menu bar of the Console panel. It has an Options menu that allows to Clear or Close the console.
	ConsoleOutput();													// Prints all the logs in the console and formats them according to their type ([ERROR], [WARNING]...)
	ConsoleScrollToBottom();											// If a new log has been added, the console will be automatically scrolled to the bottommost position.

	ImGui::End();

	return ret;
}

bool E_Console::CleanUp()
{
	bool ret = true;

	ClearLog();

	return ret;
}

void E_Console::AddLog(const char* log)
{	
	if (logs.size() + 1 > MAX_CONSOLE_LOG_SIZE)
	{
		ClearLog();
		//LOG("[WARNING] Console: Cleared Input Log: Exceeded maximum input log size!");
	}
	
	char* tmp = _strdup(log);											// strdup() duplicates the log string. This is necessary so when log is modified the console strings remain the same.

	logs.push_back(tmp);

	scroll_to_bottom = true;
}

void E_Console::ClearLog()
{
	for (uint i = 0; i < logs.size(); ++i)										// Iterates through all the strings inside logs and frees them from memory.
	{
		free(logs[i]);
	}

	logs.clear();

	scroll_to_bottom = true;
}

void E_Console::ConsoleMenuBar()
{
	ImGui::BeginMenuBar();														// Opens a menu bar on the console window so different elements can be added.

	if (ImGui::BeginMenu("Options"))											// Constructs a menu "tab" on the menu bar. Menu items can be added to the "tab".
	{
		if (ImGui::MenuItem("Clear Console"))									// Constructs a menu item inside the previously opened menu "tab".
		{
			ClearLog();
		}

		if (ImGui::MenuItem("Close Console"))
		{
			Disable();
		}

		ImGui::EndMenu();														// Closes the "Options" menu "tab".
	}

	ImGui::EndMenuBar();														// Closes the previously opened menu bar.
}

void E_Console::ConsoleOutput()
{
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 6));

	for (uint i = 0; i < logs.size(); ++i)
	{
		ImVec4 text_colour = { 1.0f, 1.0f, 1.0f, 1.0f };

		if (strstr(logs[i], "[ERROR]") != nullptr)								// strstr() will look for a specific substring in the given string. Returns nullptr if the substr is not found.
		{
			text_colour = { 1.0f, 0.33f, 0.33f, 1.0f };							// [ERROR] logs will be red.
		}
		else if (strstr(logs[i], "[WARNING]") != nullptr)
		{
			text_colour = { 1.0f, 1.0f, 0.33f, 1.0f };							// [WARNING] logs will be yellow.
		}
		else if (strstr(logs[i], "[STATUS]") != nullptr)
		{
			text_colour = { 1.0f, 0.33f, 1.0f, 1.0f };
		}
		else if (strstr(logs[i], "[FILE_SYSTEM]") != nullptr)
		{
			text_colour = { 0.33f, 0.33f, 1.0f, 1.0f };
		}
		else if (strstr(logs[i], "[IMPORTER]") != nullptr)
		{
			text_colour = { 0.33f, 1.0f, 1.0f, 1.0f };
		}
		else if (strstr(logs[i], "[SCENE]") != nullptr)
		{
			text_colour = { 0.33f, 1.0f, 0.33f, 1.0f };
		}
		else
		{
			text_colour = { 1.0f, 1.0f, 1.0f, 1.0f };
		}

		ImGui::PushStyleColor(ImGuiCol_Text, text_colour);
		ImGui::TextUnformatted(logs[i]);										// Raw text without formatting. Fast, without memory copies and no buffer size limits. For long chunks of txt.
		ImGui::PopStyleColor();
	}

	ImGui::PopStyleVar();
}

void E_Console::ConsoleScrollToBottom()
{
	if (scroll_to_bottom)
	{
		ImGui::SetScrollHereY(1.0f);											// Sets the scroll position to the given one. From 0.0f to 1.0f (From above first item to below last item).

		scroll_to_bottom = false;
	}
}