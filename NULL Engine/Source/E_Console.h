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

	char* searchStr;

	bool printErrorLogs;
	bool printWarningLogs;
	bool printStatusLogs;
	bool printFileSystemLogs;
	bool printImporterLogs;
	bool printSceneLogs;
	bool printNonTaggedLogs;

	bool scrollToBottom;
	bool showConsole;
};

#endif // !__E_CONSOLE_H__