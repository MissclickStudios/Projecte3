#include "glew/include/glew.h"						// Maybe remove later so dependencies are kept to the minimum?

#include "Time.h"

#include "Application.h"
#include "M_Window.h"
#include "M_Renderer3D.h"
#include "M_Camera3D.h"
#include "M_Input.h"
#include "M_FileSystem.h"
#include "HardwareInfo.h"

#include "GameObject.h"
#include "C_Transform.h"
#include "C_Camera.h"

#include "E_Configuration.h"

#define MAX_INPUT_LOG_SIZE 1000

E_Configuration::E_Configuration() : EditorPanel("Configuration")
{
	for (int i = 0; i < MAX_HISTOGRAM_SIZE; ++i)
	{
		fpsData[i]	= 0;
		msData[i]	= 0;
	}

	peakFps	= 0;
	minFps		= 0;
	peakMs		= 0;
	minMs		= 0;

	inputLogScrollToBottom = true;
}

E_Configuration::~E_Configuration()
{

}

bool E_Configuration::Draw(ImGuiIO& io)
{
	bool ret = true;

	ImGui::Begin(GetName(), nullptr, ImGuiWindowFlags_MenuBar);

	SetIsHovered();
	
	ConfigurationMenuBar();

	ApplicationMenu();
	WindowMenu();
	RendererMenu();
	CameraMenu();
	InputMenu();
	FileSystemMenu();
	TexturesMenu();
	AudioMenu();
	PhysicsMenu();
	SystemInfoMenu();
	TimeManagementMenu();
	ImGuiEditorsMenu();

	ImGui::End();

	return true;
}

bool E_Configuration::CleanUp()
{
	bool ret = true; 

	ClearInputLog();

	return true;
}

// ---------- ENGINE CONFIGURATION METHODS ----------
bool E_Configuration::ConfigurationMenuBar()
{
	bool ret = true;
	
	ImGui::BeginMenuBar();

	if (ImGui::BeginMenu("Options"))
	{
		if (ImGui::MenuItem("Default Settings"))
		{

		}

		if (ImGui::MenuItem("Load"))
		{

		}

		if (ImGui::MenuItem("Save"))
		{

		}

		ImGui::EndMenu();
	}

	ImGui::EndMenuBar();

	return ret;
}

bool E_Configuration::ApplicationMenu()
{
	bool ret = true;

	if (ImGui::CollapsingHeader("Application"))
	{
		char buffer[128];
		strcpy_s(buffer, App->GetEngineName());

		if (ImGui::InputText("Engine Name", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			App->SetEngineName(buffer);
		}

		PlotFrameDataHistogram();
		GenerateFrameCapSlider();
	}

	return ret;
}

bool E_Configuration::WindowMenu()
{
	bool ret = true;

	if (ImGui::CollapsingHeader("Window"))
	{
		// --- IS ACTIVE & ICON
		ImGui::Text("Is Active");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), App->window->IsActive() ? "True" : "False");
		ImGui::Text("Icon:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "None (WIP)");

		// --- WINDOW BRIGHTNESS
		GenerateBrightnessSlider();

		// --- WINDOW SIZE
		GenerateSizeSliders();

		// --- REFRESH RATE
		ImGui::Text("Refresh Rate:");  ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%d", App->window->GetRefreshRate());

		ImGui::Separator();

		// --- WINDOW MODE/FLAGS
		WindowModeFlags();
	}

	return ret;
}

bool E_Configuration::RendererMenu()
{
	bool ret = true;

	if (ImGui::CollapsingHeader("Renderer"))
	{
		// --- IS ACTIVE AND CURRENT DRIVER
		ImGui::Text("Is Active:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), App->renderer->IsActive() ? "True" : "False");
		ImGui::Text("Driver:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), App->renderer->GetDrivers());

		// --- VSYNC
		VsyncMode();

		ImGui::Separator();
		ImGui::Separator();

		// --- RENDERER FLAGS
		RendererFlags();

		ImGui::Separator();
		ImGui::Separator();

		// --- RENDERER SETTINGS
		RendererSettings();

		ImGui::Separator();
		ImGui::Separator();
	}

	return ret;
}

bool E_Configuration::CameraMenu()
{
	bool ret = true;

	if (ImGui::CollapsingHeader("Camera"))
	{
		// --- IS ACTIVE
		ImGui::Text("Is Active:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), App->camera->IsActive() ? "True" : "False");

		ImGui::SameLine(152.5f);

		if (ImGui::Button("Return to Origin"))
		{
			App->camera->ReturnToWorldOrigin();
		}

		ImGui::Separator();

		// --- TRANSFORM
		GenerateCameraPositionSlider();
		GenerateCameraRotationSlider();
		GenerateCameraScaleSlider();

		ImGui::Separator();

		// --- REFERENCE
		GenerateCameraReferenceSlider();

		ImGui::Separator();

		// --- MOVEMENT, ROTATION & ZOOM SPEED
		GenerateCameraSpeedSliders();

		ImGui::Separator();

		GenerateDrawLastRaycastCheckbox();
	}

	return ret;
}

bool E_Configuration::InputMenu()
{
	bool ret = true;

	if (ImGui::CollapsingHeader("Input"))
	{
		// --- IS ACTIVE
		ImGui::Text("Is Active:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), App->input->IsActive() ? "True" : "False");
		
		// --- MOUSE POSITION, MOTION & WHEEL
		MouseInputData();

		ImGui::Separator();

		// --- INPUT LOG
		ImGui::BeginChild("Input Log");

		InputLogOutput();
		ReduceInputLog();
		InputLogScrollToBottom();

		ImGui::EndChild();
	}

	return ret;
}

bool E_Configuration::FileSystemMenu()
{
	bool ret = true;

	if (ImGui::CollapsingHeader("File System"))
	{
		ImGui::Text("Is Active:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), App->fileSystem->IsActive() ? "True" : "False");

		GenerateBaseDirectoryText();
		GenerateReadDirectoriesText();
		GenerateWriteDirectoryText();
	}

	return ret;
}

bool E_Configuration::SystemInfoMenu()
{
	bool ret = true;

	if (ImGui::CollapsingHeader("System"))
	{
		HardwareInfo hwInfo = App->GetHardwareInfo();

		ImGui::Indent();
		
		if (ImGui::CollapsingHeader("Software"))
		{
			SDLInfo(&hwInfo);

			ImGui::Separator();

			OpenGLInfo(&hwInfo);

			ImGui::Separator();

			DevILInfo(&hwInfo);
		}

		if (ImGui::CollapsingHeader("Hardware"))
		{
			CPUInfo(&hwInfo);

			ImGui::Separator();

			GPUInfo(&hwInfo);
		}

		ImGui::Unindent();
	}
 
	return ret;
}

bool E_Configuration::TimeManagementMenu()
{
	bool ret = true;
	
	if (ImGui::CollapsingHeader("Time Management"))
	{
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Real Time Clock:");
		
		Hourglass clock				= Time::Real::GetClock();
		FrameData frameData		= Time::Real::GetFrameData();

		ImGui::Text("Time Since Start:");	ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "   %s",			clock.GetTimeAsString().c_str());
		ImGui::Text("Frame Count:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "        %llu",	frameData.frame_count);

		ImGui::Text("Average FPS:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "        %.3f",	frameData.avg_fps);
		ImGui::Text("Frames Last Second:");	ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), " %u",				frameData.frames_last_second);
		ImGui::Text("Ms Last Frame:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "      %u",		frameData.ms_last_frame);
		ImGui::Text("Delta Time:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "         %.3f",	frameData.dt);

		ImGui::Separator();

		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Game Time Clock:");

		Hourglass gameClock		= Time::Game::GetClock();
		FrameData gameFrameData	= Time::Game::GetFrameData();

		ImGui::Text("Time Since Start:");	ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "   %s",			gameClock.GetTimeAsString().c_str());
		ImGui::Text("Frame Count:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "        %llu",	gameFrameData.frame_count);

		ImGui::Text("Average FPS:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "        %.3f",	gameFrameData.avg_fps);
		ImGui::Text("Frames Last Second:");	ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), " %u",				gameFrameData.frames_last_second);
		ImGui::Text("Ms Last Frame:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "      %u",		gameFrameData.ms_last_frame);
		ImGui::Text("Delta Time:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "         %.3f",	gameFrameData.dt);
	}

	return ret;
}

bool E_Configuration::ImGuiEditorsMenu()
{
	bool ret = true;
	
	if (ImGui::CollapsingHeader("ImGui Editors"))
	{
		if (ImGui::TreeNodeEx("[IMGUI] Style Editor"))
		{
			ImGui::ShowStyleEditor();
			ImGui::TreePop();
		}

		if (ImGui::TreeNodeEx("[CUSTOM] Theme Generator"))
		{
			ImGui::ThemeGenerator(false);
			ImGui::TreePop();
		}
	}

	return ret;
}

bool E_Configuration::TexturesMenu()
{
	bool ret = true;

	if (ImGui::CollapsingHeader("Textures (WIP)"))
	{

	}

	return ret;
}

bool E_Configuration::AudioMenu()
{
	bool ret = true;

	if (ImGui::CollapsingHeader("Audio (WIP)"))
	{

	}

	return ret;
}

bool E_Configuration::PhysicsMenu()
{
	bool ret = true;

	// NOTHING FOR NOW

	return ret;
}

void E_Configuration::UpdateFrameData(int frames, int ms)
{
	for (uint i = 0; i < (MAX_HISTOGRAM_SIZE - 1); ++i)				// All elements in FPS[] and Ms[] are moved forward 1 position.
	{																// 
		fpsData[i]	= fpsData[i + 1];								// The last position is left "empty".
		msData[i]	= msData[i + 1];								// 
	}																// --------------------

	fpsData[MAX_HISTOGRAM_SIZE - 1]	= (float)frames;			// Adds to FPS[] the frames per second passed as argument to the last position in it.
	msData[MAX_HISTOGRAM_SIZE - 1]		= (float)ms;				// Adds to Ms[] the ms per frame passed as argument to the last position in it.
}

void E_Configuration::PlotFrameDataHistogram()
{
	float averageFps	= 0.0f;
	float averageMs	= 0.0f;

	for (int i = 0; i < MAX_HISTOGRAM_SIZE; ++i)
	{
		averageFps += fpsData[i];
		averageMs	+= msData[i];

		peakFps	= (peakFps < (uint)fpsData[i])	? (uint)fpsData[i] : peakFps;
		minFps		= (minFps > (uint)fpsData[i])		? (uint)fpsData[i] : minFps;
		peakMs		= (peakMs < (uint)msData[i])		? (uint)msData[i] : peakMs;
		minMs		= (minMs > (uint)msData[i])		? (uint)msData[i] : minMs;
	}

	averageFps /= (float)MAX_HISTOGRAM_SIZE;
	averageMs	/= (float)MAX_HISTOGRAM_SIZE;

	char overlay[32];
	sprintf_s(overlay, "Framerate: %.2f", fpsData[MAX_HISTOGRAM_SIZE - 1]);
	ImGui::PlotHistogram("FPS", fpsData, IM_ARRAYSIZE(fpsData), 0, overlay, 0.0f, 120.0f, ImVec2(0, 80));
	ImGui::Text("Average FPS:");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.2f", averageFps);
	ImGui::Text("Peak FPS:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "   %u", peakFps);
	ImGui::Text("Min FPS:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %u", minFps);

	sprintf_s(overlay, "ms last frame: %.2f", msData[MAX_HISTOGRAM_SIZE - 1]);
	ImGui::PlotHistogram("MS", msData, IM_ARRAYSIZE(msData), 0, overlay, 0.0f, 40.0f, ImVec2(0, 80));
	ImGui::Text("Average ms: ");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.2f", averageMs);
	ImGui::Text("Peak ms:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "   %u", peakMs);
	ImGui::Text("Min ms:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %u", minMs);
}

void E_Configuration::GenerateFrameCapSlider()
{
	int cap = App->GetFrameCap();

	ImGui::SliderInt("Frame Cap", &cap, 0, 60, "%d", ImGuiSliderFlags_AlwaysClamp);

	App->SetFrameCap(cap);

	if (cap == 0)
	{
		App->framesAreCapped = false;
	}
	else
	{
		App->framesAreCapped = true;														// [ATTENTION] Could be troubling when trying to manage the framecap elsewhere.
	}
}

void E_Configuration::GenerateBrightnessSlider()
{
	float brightness = App->window->GetBrightness();
	ImGui::SliderFloat("Brightness", &brightness, 0.250f, 1.0f, "%.3f", 0);
	App->window->SetBrightness(brightness);
}

void E_Configuration::GenerateSizeSliders()
{
	int width = 0;
	int height = 0;

	SDL_GetWindowSize(App->window->GetWindow(), &width, &height);

	uint minWidth, minHeight, maxWidth, maxHeight = 0;
	App->window->GetMinMaxSize(minWidth, minHeight, maxWidth, maxHeight);

	ImGui::SliderInt("Width", &width, minWidth, maxWidth, "%d", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderInt("Height", &height, minHeight, maxHeight, "%d", ImGuiSliderFlags_AlwaysClamp);

	App->window->SetSize(width, height);
}

void E_Configuration::WindowModeFlags()
{
	ImGui::Text("Window Mode:");

	bool maximized			= App->window->IsMaximized();
	bool fullscreen			= App->window->IsFullscreen();
	bool resizable			= App->window->IsResizable();
	bool borderless			= App->window->IsBorderless();
	bool fullscreenDesktop	= App->window->IsFullscreenDesktop();

	if (ImGui::Checkbox("Maximized", &maximized))
	{
		App->window->SetMaximized(maximized);
	}

	if (ImGui::Checkbox("Fullscreen", &fullscreen))
	{
		App->window->SetFullscreen(fullscreen);
	}

	if (ImGui::Checkbox("Resizable", &resizable))
	{
		App->window->SetResizable(resizable);
	}

	if (ImGui::Checkbox("Borderless", &borderless))
	{
		App->window->SetBorderless(borderless);
	}

	if (ImGui::Checkbox("Fullscreen Desktop", &fullscreenDesktop))
	{
		App->window->SetFullscreenDesktop(fullscreenDesktop);
	}
}

void E_Configuration::VsyncMode()
{
	bool vsync = App->renderer->GetVsync();
	if (ImGui::Checkbox("Vsync", &vsync))
	{
		App->renderer->SetVsync(vsync);
	}
}

void E_Configuration::RendererFlags()
{
	float colDist = 200.0f;
	
	ImGui::Text("Renderer flags: ");

	ImGui::Separator();
	
	// --- OPENGL FLAGS
	bool depthTest					= App->renderer->GetGLFlag(RENDERER_FLAGS::DEPTH_TEST);
	bool cullFace					= App->renderer->GetGLFlag(RENDERER_FLAGS::CULL_FACE);
	bool lighting					= App->renderer->GetGLFlag(RENDERER_FLAGS::LIGHTING);
	bool colorMaterial				= App->renderer->GetGLFlag(RENDERER_FLAGS::COLOR_MATERIAL);
	bool texture2D					= App->renderer->GetGLFlag(RENDERER_FLAGS::TEXTURE_2D);
	bool alphaTest					= App->renderer->GetGLFlag(RENDERER_FLAGS::ALPHA_TEST);
	bool blend						= App->renderer->GetGLFlag(RENDERER_FLAGS::BLEND);

	// --- SHOW FLAGS
	bool renderWorldGrid			= App->renderer->GetRenderWorldGrid();
	bool renderWorldAxis			= App->renderer->GetRenderWorldAxis();
	bool renderWireframes			= App->renderer->GetRenderWireframes();
	bool renderWertexNormals		= App->renderer->GetRenderVertexNormals();
	bool renderFaceNormals		= App->renderer->GetRenderFaceNormals();
	bool renderBoundingBoxes		= App->renderer->GetRenderBoundingBoxes();
	bool renderSkeletons			= App->renderer->GetRenderSkeletons();
	bool renderPrimitiveExamples	= App->renderer->GetRenderPrimitiveExamples();
	bool renderOthers				= false /*App->renderer->GetRenderOthers()*/;

	// --- OPENGL FLAGS
	if (ImGui::Checkbox("Depth Test", &depthTest))									{ App->renderer->SetGLFlag(RENDERER_FLAGS::DEPTH_TEST, depthTest); }			ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Cull Face", &cullFace))									{ App->renderer->SetGLFlag(RENDERER_FLAGS::CULL_FACE, cullFace); }

	if (ImGui::Checkbox("Lighting", &lighting))										{ App->renderer->SetGLFlag(RENDERER_FLAGS::LIGHTING, lighting); }				ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Color Material", &colorMaterial))							{ App->renderer->SetGLFlag(RENDERER_FLAGS::COLOR_MATERIAL, colorMaterial); }

	if (ImGui::Checkbox("Texture 2D", &texture2D))									{ App->renderer->SetGLFlag(RENDERER_FLAGS::TEXTURE_2D, texture2D); }			ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Alpha Test", &alphaTest))									{ App->renderer->SetGLFlag(RENDERER_FLAGS::ALPHA_TEST, alphaTest); }

	if (ImGui::Checkbox("Blend", &blend))											{ App->renderer->SetGLFlag(RENDERER_FLAGS::BLEND, blend); }						ImGui::SameLine(colDist);

	// --- SHOW FLAGS
	if (ImGui::Checkbox("Show World Grid", &renderWorldGrid))						{ App->renderer->SetRenderWorldGrid(renderWorldGrid); }

	if (ImGui::Checkbox("Show World Axis", &renderWorldAxis))						{ App->renderer->SetRenderWorldAxis(renderWorldAxis); }						ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Show Wireframes", &renderWireframes))						{ App->renderer->SetRenderWireframes(renderWireframes); }

	if (ImGui::Checkbox("Show Vertex Normals", &renderWertexNormals))				{ App->renderer->SetRenderVertexNormals(renderWertexNormals); }				ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Show Face Normals", &renderFaceNormals))					{ App->renderer->SetRenderFaceNormals(renderFaceNormals); }

	if (ImGui::Checkbox("Show Bounding Boxes", &renderBoundingBoxes))				{ App->renderer->SetRenderBoundingBoxes(renderBoundingBoxes); }				ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Show Skeletons", &renderSkeletons))						{ App->renderer->SetRenderSkeletons(renderSkeletons); }

	if (ImGui::Checkbox("Show Primitive Examples", &renderPrimitiveExamples))		{ App->renderer->SetRenderPrimtiveExamples(renderPrimitiveExamples); }		ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Show Others (WIP)", &renderOthers))						{ /*App->renderer->SetRenderOthers();*/ }
}

void E_Configuration::RendererSettings()
{
	float minLineWidth		= 0.1f;
	float maxLineWidth		= 10.0f;

	uint worldGridSize		= App->renderer->GetWorldGridSize();

	Color worldGridColor		= App->renderer->GetWorldGridColor(); 
	Color wireframeColor		= App->renderer->GetWireframeColor();
	Color vertexNormalsColor	= App->renderer->GetVertexNormalsColor();
	Color faceNormalsColor	= App->renderer->GetFaceNormalsColor();

	Color aabbColor			= App->renderer->GetAABBColor();
	Color obbColor				= App->renderer->GetOBBColor();
	Color frustumColor			= App->renderer->GetFrustumColor();
	Color rayColor				= App->renderer->GetRayColor();
	Color boneColor			= App->renderer->GetBoneColor();

	float worldGridLineWidth	= App->renderer->GetWorldGridLineWidth();
	float wireframeLineWidth	= App->renderer->GetWireframeLineWidth();
	float vertexNormalsWidth	= App->renderer->GetVertexNormalsWidth();
	float faceNormalsWidth	= App->renderer->GetFaceNormalsWidth();

	float aabbEdgeWidth		= App->renderer->GetAABBEdgeWidth();
	float obbEdgeWidth		= App->renderer->GetOBBEdgeWidth();
	float frustumEdgeWidth	= App->renderer->GetFrustumEdgeWidth();
	float rayWidth				= App->renderer->GetRayWidth();
	float boneWidth			= App->renderer->GetBoneWidth();
	
	ImGui::Text("Renderer Settings:");

	ImGui::Separator();

	if(ImGui::TreeNodeEx("World Grid"))
	{
		if (ImGui::ColorEdit4("W.G. Color", worldGridColor.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetWorldGridColor(worldGridColor);
		}
		if (ImGui::SliderInt("W.G. Size", (int*)&worldGridSize, 0, 420))
		{
			App->renderer->SetWorldGridSize(worldGridSize);
		}
		if (ImGui::SliderFloat("W.G. L. Width", &worldGridLineWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			App->renderer->SetWorldGridLineWidth(worldGridLineWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Wireframe"))
	{
		if (ImGui::ColorEdit4("WF. Color", wireframeColor.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetWireframeColor(wireframeColor);
		}
		if (ImGui::SliderFloat("WF. L. Width", &wireframeLineWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			App->renderer->SetWireframeLineWidth(wireframeLineWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Vertex Normals"))
	{
		if (ImGui::ColorEdit4("V.N. Color", vertexNormalsColor.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetVertexNormalsColor(vertexNormalsColor);
		}
		if (ImGui::SliderFloat("V.N. L. Width", &vertexNormalsWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			App->renderer->SetVertexNormalsWidth(vertexNormalsWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Face Normals"))
	{
		if (ImGui::ColorEdit4("F.N. Color", faceNormalsColor.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetFaceNormalsColor(faceNormalsColor);
		}
		if (ImGui::SliderFloat("F.N. L. Width", &faceNormalsWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			App->renderer->SetFaceNormalsWidth(faceNormalsWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("AABB"))
	{
		if (ImGui::ColorEdit4("AABB Color", aabbColor.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetAABBColor(aabbColor);
		}
		if (ImGui::SliderFloat("AABB E. Width", &aabbEdgeWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			App->renderer->SetAABBEdgeWidth(aabbEdgeWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("OBB"))
	{
		if (ImGui::ColorEdit4("OBB Color", obbColor.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetOBBColor(obbColor);
		}
		if (ImGui::SliderFloat("OBB E. Width", &obbEdgeWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			App->renderer->SetOBBEdgeWidth(obbEdgeWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Frustum"))
	{
		if (ImGui::ColorEdit4("Ftum Color", frustumColor.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetFrustumColor(frustumColor);
		}
		if (ImGui::SliderFloat("Ftum E. Width", &frustumEdgeWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			App->renderer->SetFrustumEdgeWidth(frustumEdgeWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Raycast"))
	{
		if (ImGui::ColorEdit4("Ray Color", rayColor.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetRayColor(rayColor);
		}
		if (ImGui::SliderFloat("Ray L. Width", &rayWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			App->renderer->SetRayWidth(rayWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Skeleton"))
	{
		if (ImGui::ColorEdit4("Bone Color", boneColor.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetBoneColor(boneColor);
		}
		if (ImGui::SliderFloat("Bone L. Width", &boneWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			App->renderer->SetBoneWidth(boneWidth);
		}

		ImGui::TreePop();
	}
}

void E_Configuration::GenerateCameraPositionSlider()
{
	float3 masterCameraPosition = App->camera->GetMasterCameraPosition();
	if (ImGui::DragFloat3("Position", (float*)&masterCameraPosition, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
	{
		App->camera->SetMasterCameraPosition(masterCameraPosition);
	}
}

void E_Configuration::GenerateCameraRotationSlider()
{
	float3 masterCameraRotation = App->camera->GetMasterCameraRotation() * RADTODEG;
	if (ImGui::DragFloat3("Rotation", (float*)&masterCameraRotation, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
	{
		App->camera->SetMasterCameraRotation(masterCameraRotation * DEGTORAD);
	}
}

void E_Configuration::GenerateCameraScaleSlider()
{
	float3 masterCameraScale = App->camera->GetMasterCameraScale();
	if (ImGui::DragFloat3("Scale", (float*)&masterCameraScale, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
	{
		App->camera->SetMasterCameraScale(masterCameraScale);
	}
}

void E_Configuration::GenerateCameraReferenceSlider()
{
	float3 cameraReference = App->camera->GetReference();
	if (ImGui::DragFloat3("Reference", (float*)&cameraReference, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
	{
		App->camera->SetReference(cameraReference);
	}
}

void E_Configuration::GenerateCameraSpeedSliders()
{
	float movementSpeed = App->camera->GetMovementSpeed();
	float rotationSpeed = App->camera->GetRotationSpeed();
	float zoomSpeed = App->camera->GetZoomSpeed();

	ImGui::DragFloat("Movement Speed", &movementSpeed, 0.01f, 0.0f, 0.0f, "%.3f", NULL);
	ImGui::DragFloat("Rotation Speed", &rotationSpeed, 0.01f, 0.0f, 0.0f, "%.3f", NULL);
	ImGui::DragFloat("Zoom Speed", &zoomSpeed, 0.01f, 0.0f, 0.0f, "%.3f", NULL);

	App->camera->SetMovementSpeed(movementSpeed);
	App->camera->SetRotationSpeed(rotationSpeed);
	App->camera->SetZoomSpeed(zoomSpeed);
}

void E_Configuration::GenerateDrawLastRaycastCheckbox()
{
	bool drawLastRaycast = App->camera->DrawLastRaycast();
	if (ImGui::Checkbox("Draw Last Raycast", &drawLastRaycast))
	{
		App->camera->SetDrawLastRaycast(drawLastRaycast);
	}
}

void E_Configuration::AddInputLog(const char* log)
{
	if (log != nullptr)
	{
		char* tmp = _strdup(log);
		
		inputLogs.push_back(tmp);

		inputLogScrollToBottom = true;
	}
}

void E_Configuration::ReduceInputLog()
{
	if (inputLogs.size() > MAX_INPUT_LOG_SIZE)
	{
		ClearInputLog();

		LOG("[WARNING] Cleared Input Log: Exceeded maximum input log size!");
	}
}

void E_Configuration::MouseInputData()
{
	ImGui::Text("Mouse Position:"); ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "x: %i y: %i", App->input->GetMouseX(), App->input->GetMouseY());
	ImGui::Text("Mouse Motion:");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "x: %i y: %i", App->input->GetMouseXMotionFromSDL(), App->input->GetMouseYMotionFromSDL());
	ImGui::Text("Mouse Wheel:");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "x: %i y: %i", App->input->GetMouseXWheel(), App->input->GetMouseYWheel());
}

void E_Configuration::InputLogOutput()
{
	for (uint i = 0; i < inputLogs.size(); ++i)
	{
		ImVec4 textColour = { 1.0f, 1.0f, 1.0f, 1.0f };									// White is the default colour, but added this to be able to easily change it.					

		if (strstr(inputLogs[i], "[KEY]") != nullptr)
		{
			textColour = { 0.0f, 1.0f, 1.0f, 1.0f };
		}
		
		if (strstr(inputLogs[i], "[MOUSE]") != nullptr)
		{
			textColour = { 1.0f, 0.0f, 1.0f, 1.0f };
		}

		ImGui::PushStyleColor(ImGuiCol_Text, textColour);
		ImGui::TextUnformatted(inputLogs[i]);
		ImGui::PopStyleColor();
	}
}

void E_Configuration::InputLogScrollToBottom()
{
	if (inputLogScrollToBottom)
	{
		ImGui::SetScrollHere(1.0f);

		inputLogScrollToBottom = false;
	}
}

void E_Configuration::ClearInputLog()
{
	for (uint i = 0; i < inputLogs.size(); ++i)
	{
		free(inputLogs[i]);
	}

	inputLogs.clear();
}

void E_Configuration::GenerateBaseDirectoryText()
{
	ImGui::Text("Base Directory:");
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	ImGui::TextWrapped(App->fileSystem->GetBaseDirectory());
	ImGui::PopStyleColor();
}

void E_Configuration::GenerateReadDirectoriesText()
{
	ImGui::Text("Read Directories:");
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	ImGui::TextWrapped(App->fileSystem->GetReadDirectories());
	ImGui::PopStyleColor();
}

void E_Configuration::GenerateWriteDirectoryText()
{
	ImGui::Text("Write Directory:");
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	ImGui::TextWrapped(App->fileSystem->GetWriteDirectory());
	ImGui::PopStyleColor();
}


void E_Configuration::SDLInfo(HardwareInfo* hw_info)
{
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "SDL Info:");

	ImGui::Text("SDL Version:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hw_info->SDL.sdl_version);
}

void E_Configuration::OpenGLInfo(HardwareInfo* hw_info)
{
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "OpenGL Info:");

	ImGui::Text("Model:");				ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hw_info->OpenGL.model_name);
	ImGui::Text("Renderer:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hw_info->OpenGL.renderer_name);
	ImGui::Text("Version:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hw_info->OpenGL.version);
	ImGui::Text("Shading Language:");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hw_info->OpenGL.shading_language_version);
}

void E_Configuration::DevILInfo(HardwareInfo* hw_info)
{
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "DevIL Info:");

	ImGui::Text("Vendor:");				ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hw_info->DevIL.vendor);
	ImGui::Text("Version:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hw_info->DevIL.version);
}

void E_Configuration::CPUInfo(HardwareInfo* hw_info)
{
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "CPU Info:");

	ImGui::Text("CPUs:");				ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%u (Cache: %ukb)", hw_info->CPU.cpu_count, hw_info->CPU.cache_size);
	ImGui::Text("RAM Size:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.1f GB", hw_info->CPU.ram_gb);

	ImGui::Text("Drivers:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s%s%s%s%s%s",
		hw_info->CPU.has_RDTSC		?	"RDTSC,"	: "",
		hw_info->CPU.has_AltiVec	?	"AltiVec,"	: "",
		hw_info->CPU.has_3DNow		?	"3DNow,"	: "",
		hw_info->CPU.has_MMX		?	"MMX,"		: "",
		hw_info->CPU.has_SSE		?	"SSE,"		: "",
		hw_info->CPU.has_SSE2		?	"SSE2,"		: "");
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s%s%s%s%s",
		hw_info->CPU.has_SSE3		?	"SSE3,"		: "",
		hw_info->CPU.has_SSE41		?	"SSE4.1,"	: "",
		hw_info->CPU.has_SSE42		?	"SSE4.2,"	: "",
		hw_info->CPU.has_RDTSC		?	"AVX,"		: "",
		hw_info->CPU.has_RDTSC		?	"AVX2,"		: "");
}

void E_Configuration::GPUInfo(HardwareInfo* hw_info)
{
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "GPU Info:");

	ImGui::Text("GPU:");				ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Vendor %i Device %i", hw_info->GPU.vendor, hw_info->GPU.device_id);
	ImGui::Text("Brand:");				ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hw_info->GPU.brand);
	ImGui::Text("VRAM Budget:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.1f MB", hw_info->GPU.vram_mb_budget);
	ImGui::Text("VRAM Usage:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.1f MB", hw_info->GPU.vram_mb_usage);
	ImGui::Text("VRAM Available:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.1f MB", hw_info->GPU.vram_mb_available);
	ImGui::Text("VRAM Reserved:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.1f MB", hw_info->GPU.vram_mb_reserved);
}