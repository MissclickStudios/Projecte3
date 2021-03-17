#include "glew/include/glew.h"						// Maybe remove later so dependencies are kept to the minimum?

#include "Time.h"
#include "Color.h"
#include "Profiler.h"
#include "EngineApplication.h"
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

#include "MemoryManager.h"

#define MAX_INPUT_LOG_SIZE 1000

E_Configuration::E_Configuration() : EditorPanel("Configuration")
{
	for (int i = 0; i < MAX_HISTOGRAM_SIZE; ++i)
	{
		fpsData[i]	= 0;
		msData[i]	= 0;
	}

	peakFps		= 0;
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
		strcpy_s(buffer, EngineApp->GetEngineName());

		if (ImGui::InputText("Engine Name", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			EngineApp->SetEngineName(buffer);
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
		ImGui::Text("Is Active");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), EngineApp->window->IsActive() ? "True" : "False");
		ImGui::Text("Icon:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "None (WIP)");

		// --- WINDOW BRIGHTNESS
		GenerateBrightnessSlider();

		// --- WINDOW SIZE
		GenerateSizeSliders();

		// --- REFRESH RATE
		ImGui::Text("Refresh Rate:");  ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%d", EngineApp->window->GetRefreshRate());

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
		ImGui::Text("Is Active:");			ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), EngineApp->renderer->IsActive() ? "True" : "False");
		ImGui::Text("Driver:");				ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), EngineApp->renderer->GetDrivers());
		ImGui::Text("Total Polycount:");	ImGui::SameLine();	ImGui::TextColored(Yellow.C_Array(), /*EngineApp->renderer->GetPolycount()*/ "0");

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
		ImGui::Text("Is Active:"); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), EngineApp->camera->IsActive() ? "True" : "False");

		ImGui::SameLine(152.5f);

		if (ImGui::Button("Return to Origin"))
		{
			EngineApp->camera->ReturnToWorldOrigin();
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
		ImGui::Text("Is Active:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), EngineApp->input->IsActive() ? "True" : "False");
		
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
		ImGui::Text("Is Active:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), EngineApp->fileSystem->IsActive() ? "True" : "False");

		GenerateBaseDirectoryText();
		GenerateReadDirectoriesText();
		GenerateWriteDirectoryText();
	}

	return ret;
}

bool E_Configuration::SystemInfoMenu()
{

	if (ImGui::CollapsingHeader("System"))
	{
		HardwareInfo hardwareInfo = EngineApp->GetHardwareInfo();

		ImGui::Indent();
		
		if (ImGui::CollapsingHeader("Software"))
		{
			SDLInfo(&hardwareInfo);

			ImGui::Separator();

			OpenGLInfo(&hardwareInfo);

			ImGui::Separator();

			DevILInfo(&hardwareInfo);
		}

		if (ImGui::CollapsingHeader("Hardware"))
		{
			CPUInfo(&hardwareInfo);

			ImGui::Separator();

			GPUInfo(&hardwareInfo);
		}

		ImGui::Unindent();
	}
 
	return true;
}

bool E_Configuration::TimeManagementMenu()
{
	if (ImGui::CollapsingHeader("Time Management"))
	{
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Real Time Clock:");
		
		Hourglass clock			= Time::Real::GetClock();
		FrameData frameData		= Time::Real::GetFrameData();

		ImGui::Text("Time Since Start:");	ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "   %s", std::string(std::to_string(clock.hours) + "h " + std::to_string(clock.minutes) + "m " + std::to_string(clock.seconds) + "s").c_str()); //TODO: can't return an std::string across dll without memo leack
		ImGui::Text("Frame Count:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "        %llu",	frameData.frameCount);

		ImGui::Text("Average FPS:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "        %.3f",	frameData.avgFps);
		ImGui::Text("Frames Last Second:");	ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), " %u",			frameData.framesLastSecond);
		ImGui::Text("Ms Last Frame:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "      %u",		frameData.msLastFrame);
		ImGui::Text("Delta Time:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "         %.3f",	frameData.dt);

		ImGui::Separator();

		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Game Time Clock:");

		Hourglass gameClock		= Time::Game::GetClock();
		FrameData gameFrameData	= Time::Game::GetFrameData();

		ImGui::Text("Time Since Start:");	ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "   %s", std::string(std::to_string(gameClock.hours) + "h " + std::to_string(gameClock.minutes) + "m " + std::to_string(gameClock.seconds) + "s").c_str()); //TODO: can't return an std::string across dll without memo leack
		ImGui::Text("Frame Count:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "        %llu",	gameFrameData.frameCount);

		ImGui::Text("Average FPS:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "        %.3f",	gameFrameData.avgFps);
		ImGui::Text("Frames Last Second:");	ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), " %u",			gameFrameData.framesLastSecond);
		ImGui::Text("Ms Last Frame:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "      %u",		gameFrameData.msLastFrame);
		ImGui::Text("Delta Time:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "         %.3f",	gameFrameData.dt);
	}

	return true;
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
	msData[MAX_HISTOGRAM_SIZE - 1]	= (float)ms;				// Adds to Ms[] the ms per frame passed as argument to the last position in it.
}

void E_Configuration::PlotFrameDataHistogram()
{
	float averageFps	= 0.0f;
	float averageMs	= 0.0f;

	for (int i = 0; i < MAX_HISTOGRAM_SIZE; ++i)
	{
		averageFps += fpsData[i];
		averageMs	+= msData[i];

		peakFps		= (peakFps < (uint)fpsData[i])	? (uint)fpsData[i]	: peakFps;
		minFps		= (minFps > (uint)fpsData[i])	? (uint)fpsData[i]	: minFps;
		peakMs		= (peakMs < (uint)msData[i])	? (uint)msData[i]	: peakMs;
		minMs		= (minMs > (uint)msData[i])		? (uint)msData[i]	: minMs;
	}

	averageFps	/= (float)MAX_HISTOGRAM_SIZE;
	averageMs	/= (float)MAX_HISTOGRAM_SIZE;

	char overlay[32];
	sprintf_s(overlay, "Framerate: %.2f", fpsData[MAX_HISTOGRAM_SIZE - 1]);
	ImGui::PlotHistogram("FPS", fpsData, IM_ARRAYSIZE(fpsData), 0, overlay, 0.0f, 144.0f, ImVec2(0, 80));
	ImGui::Text("Average FPS:");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.2f",		averageFps);
	ImGui::Text("Peak FPS:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "   %u",		peakFps);
	ImGui::Text("Min FPS:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %u",	minFps);

	sprintf_s(overlay, "ms last frame: %.2f", msData[MAX_HISTOGRAM_SIZE - 1]);
	ImGui::PlotHistogram("MS", msData, IM_ARRAYSIZE(msData), 0, overlay, 0.0f, 50.0f, ImVec2(0, 80));
	ImGui::Text("Average ms: ");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.2f",		averageMs);
	ImGui::Text("Peak ms:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "   %u",		peakMs);
	ImGui::Text("Min ms:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %u",	minMs);
}

void E_Configuration::GenerateFrameCapSlider()
{
	int cap = EngineApp->GetFrameCap();

	ImGui::SliderInt("Frame Cap", &cap, 0, 144, "%d", ImGuiSliderFlags_AlwaysClamp);

	EngineApp->SetFrameCap(cap);

	//EngineApp->framesAreCapped = (cap == 0) ? false : true;										// [ATTENTION] Could be troubling when trying to manage the framecap elsewhere.
}

void E_Configuration::GenerateBrightnessSlider()
{
	float brightness = EngineApp->window->GetBrightness();
	ImGui::SliderFloat("Brightness", &brightness, 0.250f, 1.0f, "%.3f", 0);
	EngineApp->window->SetBrightness(brightness);
}

void E_Configuration::GenerateSizeSliders()
{
	int width	= 0;
	int height	= 0;

	SDL_GetWindowSize(EngineApp->window->GetWindow(), &width, &height);

	uint minWidth, minHeight, maxWidth, maxHeight = 0;
	EngineApp->window->GetMinMaxSize(minWidth, minHeight, maxWidth, maxHeight);

	ImGui::SliderInt("Width", &width, minWidth, maxWidth, "%d", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderInt("Height", &height, minHeight, maxHeight, "%d", ImGuiSliderFlags_AlwaysClamp);

	EngineApp->window->SetSize(width, height);
}

void E_Configuration::WindowModeFlags()
{
	ImGui::Text("Window Mode:");

	bool maximized			= EngineApp->window->IsMaximized();
	bool fullscreen			= EngineApp->window->IsFullscreen();
	bool resizable			= EngineApp->window->IsResizable();
	bool borderless			= EngineApp->window->IsBorderless();
	bool fullscreenDesktop	= EngineApp->window->IsFullscreenDesktop();

	if (ImGui::Checkbox("Maximized", &maximized))
	{
		EngineApp->window->SetMaximized(maximized);
	}

	if (ImGui::Checkbox("Fullscreen", &fullscreen))
	{
		EngineApp->window->SetFullscreen(fullscreen);
	}

	if (ImGui::Checkbox("Resizable", &resizable))
	{
		EngineApp->window->SetResizable(resizable);
	}

	if (ImGui::Checkbox("Borderless", &borderless))
	{
		EngineApp->window->SetBorderless(borderless);
	}

	if (ImGui::Checkbox("Fullscreen Desktop", &fullscreenDesktop))
	{
		EngineApp->window->SetFullscreenDesktop(fullscreenDesktop);
	}
}

void E_Configuration::VsyncMode()
{
	bool vsync = EngineApp->renderer->GetVsync();
	if (ImGui::Checkbox("Vsync", &vsync))		{ EngineApp->renderer->SetVsync(vsync); }
}

void E_Configuration::RendererFlags()
{
	float colDist = 200.0f;
	
	ImGui::Text("Renderer flags: ");

	ImGui::Separator();
	
	// --- OPENGL FLAGS
	bool depthTest					= EngineApp->renderer->GetGLFlag(RendererFlags::DEPTH_TEST);
	bool cullFace					= EngineApp->renderer->GetGLFlag(RendererFlags::CULL_FACE);
	bool lighting					= EngineApp->renderer->GetGLFlag(RendererFlags::LIGHTING);
	bool colorMaterial				= EngineApp->renderer->GetGLFlag(RendererFlags::COLOR_MATERIAL);
	bool texture2D					= EngineApp->renderer->GetGLFlag(RendererFlags::TEXTURE_2D);
	bool alphaTest					= EngineApp->renderer->GetGLFlag(RendererFlags::ALPHA_TEST);
	bool blend						= EngineApp->renderer->GetGLFlag(RendererFlags::BLEND);

	// --- SHOW FLAGS
	bool renderWorldGrid			= EngineApp->renderer->GetRenderWorldGrid();
	bool renderWorldAxis			= EngineApp->renderer->GetRenderWorldAxis();
	bool renderWireframes			= EngineApp->renderer->GetRenderWireframes();
	bool renderWertexNormals		= EngineApp->renderer->GetRenderVertexNormals();
	bool renderFaceNormals			= EngineApp->renderer->GetRenderFaceNormals();
	bool renderBoundingBoxes		= EngineApp->renderer->GetRenderBoundingBoxes();
	bool renderSkeletons			= EngineApp->renderer->GetRenderSkeletons();
	bool renderPrimitiveExamples	= EngineApp->renderer->GetRenderPrimitiveExamples();
	bool renderOthers				= false /*EngineApp->renderer->GetRenderOthers()*/;

	// --- OPENGL FLAGS
	if (ImGui::Checkbox("Depth Test", &depthTest))									{ EngineApp->renderer->SetGLFlag(RendererFlags::DEPTH_TEST, depthTest); }			ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Cull Face", &cullFace))									{ EngineApp->renderer->SetGLFlag(RendererFlags::CULL_FACE, cullFace); }

	if (ImGui::Checkbox("Lighting", &lighting))										{ EngineApp->renderer->SetGLFlag(RendererFlags::LIGHTING, lighting); }				ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Color Material", &colorMaterial))							{ EngineApp->renderer->SetGLFlag(RendererFlags::COLOR_MATERIAL, colorMaterial); }

	if (ImGui::Checkbox("Texture 2D", &texture2D))									{ EngineApp->renderer->SetGLFlag(RendererFlags::TEXTURE_2D, texture2D); }			ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Alpha Test", &alphaTest))									{ EngineApp->renderer->SetGLFlag(RendererFlags::ALPHA_TEST, alphaTest); }

	if (ImGui::Checkbox("Blend", &blend))											{ EngineApp->renderer->SetGLFlag(RendererFlags::BLEND, blend); }						ImGui::SameLine(colDist);

	// --- SHOW FLAGS
	if (ImGui::Checkbox("Show World Grid", &renderWorldGrid))						{ EngineApp->renderer->SetRenderWorldGrid(renderWorldGrid); }

	if (ImGui::Checkbox("Show World Axis", &renderWorldAxis))						{ EngineApp->renderer->SetRenderWorldAxis(renderWorldAxis); }							ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Show Wireframes", &renderWireframes))						{ EngineApp->renderer->SetRenderWireframes(renderWireframes); }

	if (ImGui::Checkbox("Show Vertex Normals", &renderWertexNormals))				{ EngineApp->renderer->SetRenderVertexNormals(renderWertexNormals); }					ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Show Face Normals", &renderFaceNormals))					{ EngineApp->renderer->SetRenderFaceNormals(renderFaceNormals); }

	if (ImGui::Checkbox("Show Bounding Boxes", &renderBoundingBoxes))				{ EngineApp->renderer->SetRenderBoundingBoxes(renderBoundingBoxes); }					ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Show Skeletons", &renderSkeletons))						{ EngineApp->renderer->SetRenderSkeletons(renderSkeletons); }

	if (ImGui::Checkbox("Show Primitive Examples", &renderPrimitiveExamples))		{ EngineApp->renderer->SetRenderPrimtiveExamples(renderPrimitiveExamples); }			ImGui::SameLine(colDist);
	if (ImGui::Checkbox("Show Others (WIP)", &renderOthers))						{ /*EngineApp->renderer->SetRenderOthers();*/ }
}

void E_Configuration::RendererSettings()
{
	float minLineWidth			= 0.1f;
	float maxLineWidth			= 10.0f;

	uint worldGridSize			= EngineApp->renderer->GetWorldGridSize();

	Color worldGridColor		= EngineApp->renderer->GetWorldGridColor(); 
	Color wireframeColor		= EngineApp->renderer->GetWireframeColor();
	Color vertexNormalsColor	= EngineApp->renderer->GetVertexNormalsColor();
	Color faceNormalsColor		= EngineApp->renderer->GetFaceNormalsColor();

	Color aabbColor				= EngineApp->renderer->GetAABBColor();
	Color obbColor				= EngineApp->renderer->GetOBBColor();
	Color frustumColor			= EngineApp->renderer->GetFrustumColor();
	Color rayColor				= EngineApp->renderer->GetRayColor();
	Color boneColor				= EngineApp->renderer->GetBoneColor();

	float worldGridLineWidth	= EngineApp->renderer->GetWorldGridLineWidth();
	float wireframeLineWidth	= EngineApp->renderer->GetWireframeLineWidth();
	float vertexNormalsWidth	= EngineApp->renderer->GetVertexNormalsWidth();
	float faceNormalsWidth		= EngineApp->renderer->GetFaceNormalsWidth();

	float aabbEdgeWidth			= EngineApp->renderer->GetAABBEdgeWidth();
	float obbEdgeWidth			= EngineApp->renderer->GetOBBEdgeWidth();
	float frustumEdgeWidth		= EngineApp->renderer->GetFrustumEdgeWidth();
	float rayWidth				= EngineApp->renderer->GetRayWidth();
	float boneWidth				= EngineApp->renderer->GetBoneWidth();
	
	ImGui::Text("Renderer Settings:");

	ImGui::Separator();

	if(ImGui::TreeNodeEx("World Grid"))
	{
		if (ImGui::ColorEdit4("W.G. Color", worldGridColor.C_Array(), ImGuiColorEditFlags_None))
		{
			EngineApp->renderer->SetWorldGridColor(worldGridColor);
		}
		if (ImGui::SliderInt("W.G. Size", (int*)&worldGridSize, 0, 420))
		{
			EngineApp->renderer->SetWorldGridSize(worldGridSize);
		}
		if (ImGui::SliderFloat("W.G. L. Width", &worldGridLineWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			EngineApp->renderer->SetWorldGridLineWidth(worldGridLineWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Wireframe"))
	{
		if (ImGui::ColorEdit4("WF. Color", wireframeColor.C_Array(), ImGuiColorEditFlags_None))
		{
			EngineApp->renderer->SetWireframeColor(wireframeColor);
		}
		if (ImGui::SliderFloat("WF. L. Width", &wireframeLineWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			EngineApp->renderer->SetWireframeLineWidth(wireframeLineWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Vertex Normals"))
	{
		if (ImGui::ColorEdit4("V.N. Color", vertexNormalsColor.C_Array(), ImGuiColorEditFlags_None))
		{
			EngineApp->renderer->SetVertexNormalsColor(vertexNormalsColor);
		}
		if (ImGui::SliderFloat("V.N. L. Width", &vertexNormalsWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			EngineApp->renderer->SetVertexNormalsWidth(vertexNormalsWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Face Normals"))
	{
		if (ImGui::ColorEdit4("F.N. Color", faceNormalsColor.C_Array(), ImGuiColorEditFlags_None))
		{
			EngineApp->renderer->SetFaceNormalsColor(faceNormalsColor);
		}
		if (ImGui::SliderFloat("F.N. L. Width", &faceNormalsWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			EngineApp->renderer->SetFaceNormalsWidth(faceNormalsWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("AABB"))
	{
		if (ImGui::ColorEdit4("AABB Color", aabbColor.C_Array(), ImGuiColorEditFlags_None))
		{
			EngineApp->renderer->SetAABBColor(aabbColor);
		}
		if (ImGui::SliderFloat("AABB E. Width", &aabbEdgeWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			EngineApp->renderer->SetAABBEdgeWidth(aabbEdgeWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("OBB"))
	{
		if (ImGui::ColorEdit4("OBB Color", obbColor.C_Array(), ImGuiColorEditFlags_None))
		{
			EngineApp->renderer->SetOBBColor(obbColor);
		}
		if (ImGui::SliderFloat("OBB E. Width", &obbEdgeWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			EngineApp->renderer->SetOBBEdgeWidth(obbEdgeWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Frustum"))
	{
		if (ImGui::ColorEdit4("Ftum Color", frustumColor.C_Array(), ImGuiColorEditFlags_None))
		{
			EngineApp->renderer->SetFrustumColor(frustumColor);
		}
		if (ImGui::SliderFloat("Ftum E. Width", &frustumEdgeWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			EngineApp->renderer->SetFrustumEdgeWidth(frustumEdgeWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Raycast"))
	{
		if (ImGui::ColorEdit4("Ray Color", rayColor.C_Array(), ImGuiColorEditFlags_None))
		{
			EngineApp->renderer->SetRayColor(rayColor);
		}
		if (ImGui::SliderFloat("Ray L. Width", &rayWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			EngineApp->renderer->SetRayWidth(rayWidth);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Skeleton"))
	{
		if (ImGui::ColorEdit4("Bone Color", boneColor.C_Array(), ImGuiColorEditFlags_None))
		{
			EngineApp->renderer->SetBoneColor(boneColor);
		}
		if (ImGui::SliderFloat("Bone L. Width", &boneWidth, minLineWidth, maxLineWidth, "%.3f", 1.0f))
		{
			EngineApp->renderer->SetBoneWidth(boneWidth);
		}

		ImGui::TreePop();
	}
}

void E_Configuration::GenerateCameraPositionSlider()
{
	float3 masterCameraPosition = EngineApp->camera->GetMasterCameraPosition();
	if (ImGui::DragFloat3("Position", (float*)&masterCameraPosition, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
	{
		EngineApp->camera->SetMasterCameraPosition(masterCameraPosition);
	}
}

void E_Configuration::GenerateCameraRotationSlider()
{
	float3 masterCameraRotation = EngineApp->camera->GetMasterCameraRotation() * RADTODEG;
	if (ImGui::DragFloat3("Rotation", (float*)&masterCameraRotation, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
	{
		EngineApp->camera->SetMasterCameraRotation(masterCameraRotation * DEGTORAD);
	}
}

void E_Configuration::GenerateCameraScaleSlider()
{
	float3 masterCameraScale = EngineApp->camera->GetMasterCameraScale();
	if (ImGui::DragFloat3("Scale", (float*)&masterCameraScale, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
	{
		EngineApp->camera->SetMasterCameraScale(masterCameraScale);
	}
}

void E_Configuration::GenerateCameraReferenceSlider()
{
	float3 cameraReference = EngineApp->camera->GetReference();
	if (ImGui::DragFloat3("Reference", (float*)&cameraReference, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
	{
		EngineApp->camera->SetReference(cameraReference);
	}
}

void E_Configuration::GenerateCameraSpeedSliders()
{
	float movementSpeed = EngineApp->camera->GetMovementSpeed();
	float rotationSpeed = EngineApp->camera->GetRotationSpeed();
	float zoomSpeed = EngineApp->camera->GetZoomSpeed();

	ImGui::DragFloat("Movement Speed", &movementSpeed, 0.01f, 0.0f, 0.0f, "%.3f", NULL);
	ImGui::DragFloat("Rotation Speed", &rotationSpeed, 0.01f, 0.0f, 0.0f, "%.3f", NULL);
	ImGui::DragFloat("Zoom Speed", &zoomSpeed, 0.01f, 0.0f, 0.0f, "%.3f", NULL);

	EngineApp->camera->SetMovementSpeed(movementSpeed);
	EngineApp->camera->SetRotationSpeed(rotationSpeed);
	EngineApp->camera->SetZoomSpeed(zoomSpeed);
}

void E_Configuration::GenerateDrawLastRaycastCheckbox()
{
	bool drawLastRaycast = EngineApp->camera->DrawLastRaycast();
	if (ImGui::Checkbox("Draw Last Raycast", &drawLastRaycast))
	{
		EngineApp->camera->SetDrawLastRaycast(drawLastRaycast);
	}
}

//Redo
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
	ImGui::Text("Mouse Position:"); ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "x: %i y: %i", EngineApp->input->GetMouseX(), EngineApp->input->GetMouseY());
	ImGui::Text("Mouse Motion:");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "x: %i y: %i", EngineApp->input->GetMouseXMotionFromSDL(), EngineApp->input->GetMouseYMotionFromSDL());
	ImGui::Text("Mouse Wheel:");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "x: %i y: %i", EngineApp->input->GetMouseXWheel(), EngineApp->input->GetMouseYWheel());
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
	ImGui::TextWrapped(EngineApp->fileSystem->GetBaseDirectory());
	ImGui::PopStyleColor();
}

void E_Configuration::GenerateReadDirectoriesText()
{
	ImGui::Text("Read Directories:");
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	ImGui::TextWrapped(EngineApp->fileSystem->GetReadDirectories());
	ImGui::PopStyleColor();
}

void E_Configuration::GenerateWriteDirectoryText()
{
	ImGui::Text("Write Directory:");
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	ImGui::TextWrapped(EngineApp->fileSystem->GetWriteDirectory());
	ImGui::PopStyleColor();
}


void E_Configuration::SDLInfo(HardwareInfo* hardwareInfo)
{
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "SDL Info:");

	ImGui::Text("SDL Version:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hardwareInfo->SDL.SDLVersion);
}

void E_Configuration::OpenGLInfo(HardwareInfo* hardwareInfo)
{
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "OpenGL Info:");

	ImGui::Text("Model:");				ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hardwareInfo->OpenGL.modelName);
	ImGui::Text("Renderer:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hardwareInfo->OpenGL.rendererName);
	ImGui::Text("Version:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hardwareInfo->OpenGL.version);
	ImGui::Text("Shading Language:");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hardwareInfo->OpenGL.shadingLanguageVersion);
}

void E_Configuration::DevILInfo(HardwareInfo* hardwareInfo)
{
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "DevIL Info:");

	ImGui::Text("Vendor:");				ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hardwareInfo->DevIL.vendor);
	ImGui::Text("Version:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s", hardwareInfo->DevIL.version);
}

void E_Configuration::CPUInfo(HardwareInfo* hardwareInfo)
{
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "CPU Info:");

	ImGui::Text("CPUs:");				ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%u (Cache: %ukb)", hardwareInfo->CPU.cpuCount, hardwareInfo->CPU.cacheSize);
	ImGui::Text("RAM Size:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.1f GB", hardwareInfo->CPU.ramGb);

	ImGui::Text("Drivers:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s%s%s%s%s%s",
		hardwareInfo->CPU.hasRDTSC		?	"RDTSC,"	: "",
		hardwareInfo->CPU.hasAltiVec	?	"AltiVec,"	: "",
		hardwareInfo->CPU.has3DNow		?	"3DNow,"	: "",
		hardwareInfo->CPU.hasMMX		?	"MMX,"		: "",
		hardwareInfo->CPU.hasSSE		?	"SSE,"		: "",
		hardwareInfo->CPU.hasSSE2		?	"SSE2,"		: "");
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s%s%s%s%s",
		hardwareInfo->CPU.hasSSE3		?	"SSE3,"		: "",
		hardwareInfo->CPU.hasSSE41		?	"SSE4.1,"	: "",
		hardwareInfo->CPU.hasSSE42		?	"SSE4.2,"	: "",
		hardwareInfo->CPU.hasRDTSC		?	"AVX,"		: "",
		hardwareInfo->CPU.hasRDTSC		?	"AVX2,"		: "");
}

void E_Configuration::GPUInfo(HardwareInfo* hardwareInfo)
{
	ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "GPU Info:");

	ImGui::Text("GPU:");				ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Vendor %i Device %i",	hardwareInfo->GPU.vendor, hardwareInfo->GPU.deviceId);
	ImGui::Text("Brand:");				ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%s",					hardwareInfo->GPU.brand);
	ImGui::Text("VRAM Budget:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.1f MB",				hardwareInfo->GPU.vramBudget);
	ImGui::Text("VRAM Usage:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.1f MB",				hardwareInfo->GPU.vramUsage);
	ImGui::Text("VRAM Available:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.1f MB",				hardwareInfo->GPU.vramAvailable);
	ImGui::Text("VRAM Reserved:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.1f MB",				hardwareInfo->GPU.vramReserved);
}