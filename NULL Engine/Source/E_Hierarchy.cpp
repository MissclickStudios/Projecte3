#include "EngineApplication.h"
#include "M_Editor.h"

#include "GameObject.h"

#include "E_Hierarchy.h"

#include "MemoryManager.h"

E_Hierarchy::E_Hierarchy() : EditorPanel("Hierarchy"), 
draggedGameObject			(nullptr), 
openHierarchyToolsPopup	(false)
{
	defaultFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
}

E_Hierarchy::~E_Hierarchy()
{

}

bool E_Hierarchy::Draw(ImGuiIO& io)
{
	bool ret = true;

	ImGui::Begin("Hierarchy");

	SetIsHovered();

	PrintGameObjectsOnHierarchy();

	ImGui::End();

	return ret;
}

bool E_Hierarchy::CleanUp()
{
	bool ret = true;

	return ret;
}

void E_Hierarchy::PrintGameObjectsOnHierarchy()
{	
	if (EngineApp->editor->GetSceneRootThroughEditor() != nullptr)
	{
		if (openHierarchyToolsPopup)
		{
			HierarchyToolsPopup();
		}
		
		ProcessGameObject(EngineApp->editor->GetSceneRootThroughEditor());
	}
}

void E_Hierarchy::ProcessGameObject(GameObject* gameObject)
{	
	// ------ Setting the tree node's color. ------
	ImVec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
	
	if (!gameObject->IsActive())														// If the given game object is not active, the text of the tree node will be displayed in GREY.
	{
		color = { 0.5f, 0.5f, 0.5f, 1.0f };
	}
	
	ImGui::PushStyleColor(ImGuiCol_Text, color);
	// --------------------------------------------

	ImGuiTreeNodeFlags nodeFlags = defaultFlags;

	if (gameObject->childs.empty())
	{
		nodeFlags |= ImGuiTreeNodeFlags_Leaf;
	}

	if (gameObject == EngineApp->editor->GetSelectedGameObjectThroughEditor())
	{
		nodeFlags |= ImGuiTreeNodeFlags_Selected;
	}

	if (gameObject == EngineApp->editor->GetSceneRootThroughEditor())
	{
		nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
	}

	if (ImGui::TreeNodeEx(gameObject->GetName(), nodeFlags))
	{
		if (!NodeIsRootObject(gameObject))													// If the game_object being processed is the root object, do not allow any interaction.
		{
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))								// IsItemClicked() checks if the TreeNode item was clicked.
			{																				// Arguments:
				EngineApp->editor->SetSelectedGameObjectThroughEditor(gameObject);				// 0 = Left Click
			}																				// 1 = Right Click

			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))								// 
			{																				// 
				EngineApp->editor->SetSelectedGameObjectThroughEditor(gameObject);				// 
				openHierarchyToolsPopup = true;											// 
			}																				// -----------------------------------------------------------------------------------------------

			if (ImGui::BeginDragDropSource())												// First, it is checked whether or not this node is part of a currently starting drag&drop operation.
			{
				ImGui::SetDragDropPayload("DRAGGED_NODE", gameObject, sizeof(GameObject));	// Here the payload is being constructed. It can be later identified through the given string.
				ImGui::Text("Dragging %s", gameObject->GetName());							// This specific text, as it is within the DragDropSource, will accompany the dragged node.
				draggedGameObject = gameObject;											// The dragged game object needs to be saved to be later re-integrated into the hierarchy.

				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())												// Here it is checked whether or not an element is being dropped into this specific node/item.
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAGGED_NODE"))	// First, the payload that is being dropped needs to be checked to make sure its the correct one.
				{
					//game_object->AddChild(dragged_game_object);								// (GameObject*)payload->Data would also work. However, it easily breaks, at least in my case.

					draggedGameObject->SetParent(gameObject);
					draggedGameObject = nullptr;
				}

				ImGui::EndDragDropTarget();
			}
		}

		if (!gameObject->childs.empty())
		{
			for (uint i = 0; i < gameObject->childs.size(); ++i)
			{
				ProcessGameObject(gameObject->childs[i]);
			}
		}

		ImGui::TreePop();
	}

	ImGui::PopStyleColor();
}

void E_Hierarchy::HierarchyToolsPopup()
{
	ImGui::OpenPopup("Hierarchy Tools");
	if (ImGui::BeginPopup("Hierarchy Tools"))
	{
		if (ImGui::MenuItem("Create Empty Child GameObject"))
		{
			EngineApp->editor->CreateGameObject("Empty Child", EngineApp->editor->GetSelectedGameObjectThroughEditor());
			openHierarchyToolsPopup = false;
		}
		
		if (ImGui::MenuItem("Delete Selected"))
		{
			if (SelectedCanBeDeleted())
			{
				EngineApp->editor->DeleteSelectedGameObject();
				openHierarchyToolsPopup = false;
			}
		}

		ImGui::EndPopup();
	}

	if (ImGui::IsMouseReleased(0))
	{
		openHierarchyToolsPopup = false;
	}
}

bool E_Hierarchy::NodeIsRootObject(GameObject* node)
{
	return node == EngineApp->editor->GetSceneRootThroughEditor();
}

bool E_Hierarchy::SelectedCanBeDeleted()
{
	if (EngineApp->editor->SelectedIsSceneRoot())
	{
		LOG("[WARNING] Hierarchy: The Scene's Root Object cannot be deleted!");
	}
	if (EngineApp->editor->SelectedIsAnimationBone())
	{
		LOG("[WARNING] Herarchy: An Animation Bone GameObject cannot be deleted! Delete Animation Parent Game Object First.");
	}

	return (!EngineApp->editor->SelectedIsSceneRoot() && !EngineApp->editor->SelectedIsAnimationBone());
}