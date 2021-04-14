#ifndef __E_WANT_TO_SAVE_SCENE_H__
#define __E_WANT_TO_SAVE_SCENE_H__

class EditorPanel;

enum class WantToSaveType
{
	NEW_SCENE,
	OPEN_SCENE,
	QUIT,
	NONE
};

class E_WantToSaveScene : public EditorPanel
{
public:
	E_WantToSaveScene();
	~E_WantToSaveScene();

	bool Draw(ImGuiIO& io);
	bool CleanUp();

	void Save();
	void DontSave();
	void Cancel();

private:
	bool		readyToSave;
	std::string	sceneName;

public:
	WantToSaveType type = WantToSaveType::NONE;
};

#endif // !__E_SAVE_FILE_H__
