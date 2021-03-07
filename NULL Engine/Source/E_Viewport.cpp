#include "MathGeoTransform.h"
#include "MathGeoFrustum.h"

#include "EngineApplication.h"
#include "M_Window.h"
#include "M_Renderer3D.h"
#include "M_Input.h"
#include "M_Editor.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Camera.h"

#include "E_Viewport.h"

E_Viewport::E_Viewport() : EditorPanel("Viewport"),
texSize			({ 0.0f, 0.0f }),
texOrigin		({ 0.0f, 0.0f }),
cursorPos		({ 0.0f, 0.0f }),
guizmoOperation	(ImGuizmo::OPERATION::TRANSLATE),
guizmoMode		(ImGuizmo::MODE::WORLD),
usingGuizmo		(false),
sceneFocused	(false),
gameFocused		(false),
wantTextInput	(false)
{

}

E_Viewport::~E_Viewport()
{

}

bool E_Viewport::Draw(ImGuiIO& io)
{
	bool ret = true;
	
	wantTextInput = io.WantTextInput;																	// Later Generalize as a M_Editor Variable/Method.
	
	DrawScene();
	
	DrawGame();

	return ret;
}

bool E_Viewport::CleanUp()
{
	bool ret = true;



	return ret;
}

// --- E_SCENE METHODS ---
float2 E_Viewport::GetWorldMousePosition()
{	
	float winWidth		= (float)EngineApp->window->GetWidth();
	float winHeight		= (float)EngineApp->window->GetHeight();
	
	float texWidth		= texSize.x;
	float texHeight		= texSize.y;

	float mouseX		= (float)EngineApp->input->GetMouseX();
	float mouseY		= (float)EngineApp->input->GetMouseY();

	float2 screenMousePos = float2(mouseX, winHeight - mouseY) - float2(texOrigin.x, texOrigin.y + 22.5f);				// TMP. Un-Hardcode Later.
	//float2 screen_mouse_pos = GetScreenMousePosition();
	float2 normScreenPos	= float2(screenMousePos.x / texWidth, screenMousePos.y / texHeight);
	float2 worldMousePos	= float2(normScreenPos.x * winWidth, normScreenPos.y * winHeight);

	return worldMousePos;
}

float2 E_Viewport::GetScreenMousePosition()
{
	float winWidth		= (float)EngineApp->window->GetWidth();
	float winHeight	= (float)EngineApp->window->GetHeight();

	float texWidth		= texSize.x;
	float texHeight		= texSize.y;

	float mouseX		= (float)EngineApp->input->GetMouseX();
	float mouseY		= (float)EngineApp->input->GetMouseY();

	float2 worldMousePos	= float2(mouseX, mouseY);
	float2 normWorldPos	= float2(worldMousePos.x / winWidth, worldMousePos.y / winHeight);
	float2 screenMousePos = float2(normWorldPos.x * texWidth, normWorldPos.y * texHeight);

	screenMousePos		+= float2(texOrigin.x, texOrigin.y);

	return screenMousePos;
}

float2 E_Viewport::GetWorldMouseMotion()
{
	float2 winMouseMotion	= float2((float)EngineApp->input->GetMouseXMotion(), (float)EngineApp->input->GetMouseYMotion());
	float2 winSize			= float2((float)EngineApp->window->GetWidth(), (float)EngineApp->window->GetHeight());
	float2 texSize			= float2(this->texSize.x, this->texSize.y);

	float2 localMotion			= float2(winMouseMotion.x / texSize.x, winMouseMotion.y / texSize.y);
	float2 worldMouseMotion	= float2(localMotion.x * winSize.x, localMotion.y * winSize.y);

	return worldMouseMotion;
}

float2 E_Viewport::GetSceneTextureSize()
{
	return float2(texSize.x, texSize.y);
}

bool E_Viewport::UsingGuizmo()
{
	return ImGuizmo::IsUsing();
}

bool E_Viewport::HoveringGuizmo()
{
	return /*ImGuizmo::IsUsing ||*/ ImGuizmo::IsOver();
}

void E_Viewport::DrawScene()
{
	ImGui::Begin("Scene");

	if ((EngineApp->gameState != GameState::PLAY) && !sceneFocused)
	{
		ImGui::FocusWindow(ImGui::GetCurrentWindow());
		ImGui::FocusWindow(nullptr);
		
		sceneFocused	= true;
		gameFocused		= false;
	}
	
	if (sceneFocused)
	{
		SetIsHovered();
	}

	AdaptTextureToWindowSize();

	DrawSceneTexture();

	HandleGuizmos();

	ResourceDragAndDropTargetListener();
}

void E_Viewport::DrawGame()
{
	ImGui::Begin("Game", (bool*)0, ImGuiWindowFlags_NoFocusOnAppearing);

	if ((EngineApp->gameState == GameState::PLAY) && !gameFocused)
	{
		ImGui::FocusWindow(ImGui::GetCurrentWindow());
		ImGui::FocusWindow(nullptr);

		gameFocused		= true;
		sceneFocused	= false;
	}

	if (gameFocused)
	{
		SetIsHovered();
	}

	AdaptTextureToWindowSize();

	DrawSceneTexture();

	HandleGuizmos();

	ImGui::End();
}

void E_Viewport::AdaptTextureToWindowSize()
{	
	texSize				= ImVec2((float)EngineApp->window->GetWidth(), (float)EngineApp->window->GetHeight());
	ImVec2 winSize		= ImGui::GetWindowSize() * 0.975f;													// An offset is added so the image is entirely enclosed by the window.

	float widthRatio	= (texSize.x / winSize.x);														// tex.x to win.x ratio and tex.y to win.y ratio.
	float heightRatio	= (texSize.y / winSize.y);														// Will be used to maintain the aspect ratio of the tex after win is resized.

	if (texSize.x > winSize.x)
	{
		texSize = texSize / widthRatio;																	// Will adapt the tex to the width ratio between the tex and the win widths.
	}

	if (texSize.y > winSize.y)
	{
		texSize = texSize / heightRatio;																	// Will adapt the tex to the height ratio between the tex and the win heights.
	}
}

void E_Viewport::DrawSceneTexture()
{
	cursorPos = (ImGui::GetWindowSize() - texSize) * 0.5f;
	ImGui::SetCursorPos(cursorPos);

	ImVec2 screenCursorPos = ImGui::GetCursorScreenPos();

	if (screenCursorPos.x > 1920)																			// Need to take into account having multiple screens. Un-Harcode Later!
	{
		screenCursorPos.x = screenCursorPos.x - 1920;
	}

	texOrigin	= screenCursorPos + ImVec2(0, texSize.y);											// Getting the top-left corner at XY.
	texOrigin.y	= (float)EngineApp->window->GetHeight() - texOrigin.y;										// Converting from top-left Y origin to bottom-left Y origin.

	ImGui::Image((ImTextureID)EngineApp->renderer->GetSceneRenderTexture(), texSize, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
	//ImGui::Image((ImTextureID)EngineApp->renderer->GetDepthBufferTexture(), tex_size, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
}

void E_Viewport::ResourceDragAndDropTargetListener()
{
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAGGED_ASSET"))
		{
			EngineApp->editor->LoadResourceIntoSceneThroughEditor();
		}

		ImGui::EndDragDropTarget();
	}

	ImGui::End();
}

void E_Viewport::HandleGuizmos()
{	
	usingGuizmo = false;

	if (!wantTextInput)
	{
		if (EngineApp->input->GetKey(SDL_SCANCODE_W) == KeyState::KEY_DOWN)
		{
			guizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
			guizmoMode = ImGuizmo::MODE::WORLD;
		}
		if (EngineApp->input->GetKey(SDL_SCANCODE_E) == KeyState::KEY_DOWN)
		{
			guizmoOperation = ImGuizmo::OPERATION::ROTATE;
			guizmoMode = ImGuizmo::MODE::WORLD;
		}
		if (EngineApp->input->GetKey(SDL_SCANCODE_R) == KeyState::KEY_DOWN)
		{
			guizmoOperation = ImGuizmo::OPERATION::SCALE;
			guizmoMode = ImGuizmo::MODE::LOCAL;
		}
	}
	
	if (EngineApp->editor->GetSelectedGameObjectThroughEditor() == nullptr)
		return;

	if (EngineApp->editor->GetCurrentCameraThroughEditor() == nullptr)
		return;
	
	GameObject* selected		= EngineApp->editor->GetSelectedGameObjectThroughEditor();
	C_Camera* currentCamera	= EngineApp->editor->GetCurrentCameraThroughEditor();

	float4x4 viewMatrix		= currentCamera->GetFrustum().ViewMatrix();
	float4x4 projectionMatrix	= currentCamera->GetFrustum().ProjectionMatrix();
	float4x4 worldTransform	= selected->GetComponent<C_Transform>()->GetWorldTransform();
	viewMatrix.Transpose();																				// MathGeoLib works with Row-Major matrices and ImGuizmo works with
	projectionMatrix.Transpose();																			// Column-Major matrices. Hence the need to transpose them.
	worldTransform.Transpose();																			// ----------------------------------------------------------------

	ImGuizmo::SetDrawlist();

	float winHeight		= (float)EngineApp->window->GetHeight();
	float texWidth		= texSize.y;
	ImVec2 originPos	= ImVec2(texOrigin.x, winHeight - texOrigin.y - texWidth);

	ImGuizmo::SetRect(texOrigin.x, originPos.y, texSize.x, texSize.y);

	ImGuizmo::Manipulate(viewMatrix.ptr(), projectionMatrix.ptr(), guizmoOperation, guizmoMode, worldTransform.ptr());

	if (ImGuizmo::IsUsing())
	{
		usingGuizmo = true;

		worldTransform = worldTransform.Transposed();
		selected->GetComponent<C_Transform>()->SetWorldTransform(worldTransform);
	}
}
