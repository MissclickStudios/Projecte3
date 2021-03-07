#ifndef __E_SAVE_FILE_H__
#define __E_SAVE_FILE_H__

#include "EditorPanel.h"

class E_SaveFile : public EditorPanel
{
public:
	E_SaveFile();
	~E_SaveFile();

	bool Draw(ImGuiIO& io);
	bool CleanUp();

private:
	void DrawTextEditor();


private:
	bool		readyToSave;
	std::string	sceneName;
};

#endif // !__E_SAVE_FILE_H__
