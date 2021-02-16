#ifndef __E_HIERARCHY_H__
#define __E_HIERARCHY_H__

#include "EditorPanel.h"

class GameObject;

class E_Hierarchy : public EditorPanel
{
public:
	E_Hierarchy();
	~E_Hierarchy();

	bool Draw		(ImGuiIO& io) override;
	bool CleanUp	() override;

private:
	void PrintGameObjectsOnHierarchy	();											// Will process all the GameObjects in the scene through the root and print their corresponding tree nodes on the hierarchy.
	void ProcessGameObject				(GameObject* game_object);					// Will process the given GameObject and recursively process its childs as well.

	void HierarchyToolsPopup			();											// Will generate a modal window with the hierarchy tools in it.

	bool NodeIsRootObject				(GameObject* node);

	bool SelectedCanBeDeleted			();

private:
	ImGuiTreeNodeFlags	default_flags;												// Will define the base configuration of the tree nodes.
	GameObject*			dragged_game_object;										// Used for the dragging and dropping of game objects withing the hierarchy.

	bool				open_hierarchy_tools_popup;									// Will keep track of whether or not the hierarchy tools popup needs to be shown.
};

#endif // !__E_HIERARCHY_H__