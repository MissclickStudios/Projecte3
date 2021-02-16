#ifndef __E_LOAD_FILE_H__
#define __E_LOAD_FILE_H__

#include "EditorPanel.h"

class E_LoadFile : public EditorPanel
{
public:
	E_LoadFile();
	~E_LoadFile();

	bool Draw		(ImGuiIO& io);
	bool CleanUp	();

private:
	void DrawFileBrowser		();																		// Will diplay a child window with all the directories and files inside "Assets/"
	void DrawFileSelector		();																		// Will display an input box with the selected_file in it and a Confirm & Cancel btns.
	
	void DrawDirectoriesTree	(const char* rootDirectory, const char* extensionToFilter);			// Will display all the directories and files inside the root directory.

private:
	char*	selectedFile;																				// String that contains the path of the currently selected file.
	bool	readyToLoad;																				// Will keep track of whether or not scene_file is ready to be sent to Load().

};

#endif // !__E_LOAD_FILE_H__