#ifndef __E_RESOURCES_H__
#define __E_RESOURCES_H__

#include "EditorPanel.h"

class E_Resources : public EditorPanel
{
public:
	E_Resources();
	~E_Resources();

	bool Draw(ImGuiIO& io);
	bool CleanUp();

private:
	void DrawModelsHeader		();
	void DrawMeshesHeader		();
	void DrawMaterialsHeader	();
	void DrawTexturesHeader		();
	void DrawFoldersHeader		();
	void DrawAnimationsHeader	();
	void DrawScenesHeader		();
};

#endif // !__E_RESOURCES_H__