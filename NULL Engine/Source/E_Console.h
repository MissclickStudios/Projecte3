#ifndef __E_CONSOLE_H__
#define __E_CONSOLE_H__

#include "EditorPanel.h"

class E_Console : public EditorPanel
{
public:
	E_Console();
	~E_Console();

	bool Draw(ImGuiIO& io) override;

	bool CleanUp() override;

public:
	void AddLog(const char* log);
	
private:
	void ClearLog();

	void ConsoleMenuBar();
	void ConsoleOutput();
	void ConsoleScrollToBottom();
	
private:
	std::vector<char*> logs;

	bool scroll_to_bottom;
	bool show_console;
};

#endif // !__E_CONSOLE_H__