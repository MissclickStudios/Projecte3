#ifndef __E_VIEWPORT_H__
#define __E_VIEWPORT_H__

#include "MathGeoLib/include/Math/float2.h"
#include "ImGuizmo/include/ImGuizmo.h"
#include "EditorPanel.h"

class E_Viewport : public EditorPanel
{
public:
	E_Viewport();
	~E_Viewport();

	bool Draw			(ImGuiIO& io) override;
	bool CleanUp		() override;

public:
	float2	GetWorldMousePosition				();
	float2	GetScreenMousePosition				();
	float2	GetWorldMouseMotion					();
	float2	GetSceneTextureSize					();

	bool	UsingGuizmo							();
	bool	HoveringGuizmo						();

private:
	void	DrawScene							();
	void	DrawGame							();
	
	void	AdaptTextureToWindowSize			();
	void	DrawSceneTexture					();

	void	ResourceDragAndDropTargetListener	();

	void	HandleGuizmos						();

private:
	ImVec2					tex_size;
	ImVec2					tex_origin;
	ImVec2					cursor_pos;

	ImGuizmo::OPERATION		guizmo_operation;
	ImGuizmo::MODE			guizmo_mode;

	bool					using_guizmo;

	bool					scene_focused;
	bool					game_focused;

private:
	bool					want_text_input;
};

#endif // !__E_VIEWPORT_H__