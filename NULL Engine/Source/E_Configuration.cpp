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
		FPS_data[i]	= 0;
		ms_data[i]	= 0;
	}

	peak_FPS	= 0;
	min_FPS		= 0;
	peak_ms		= 0;
	min_ms		= 0;

	input_log_scroll_to_bottom = true;
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
		ImGui::Text("Is Active:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), App->file_system->IsActive() ? "True" : "False");

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
		HardwareInfo hw_info = App->GetHardwareInfo();

		ImGui::Indent();
		
		if (ImGui::CollapsingHeader("Software"))
		{
			SDLInfo(&hw_info);

			ImGui::Separator();

			OpenGLInfo(&hw_info);

			ImGui::Separator();

			DevILInfo(&hw_info);
		}

		if (ImGui::CollapsingHeader("Hardware"))
		{
			CPUInfo(&hw_info);

			ImGui::Separator();

			GPUInfo(&hw_info);
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
		FrameData frame_data		= Time::Real::GetFrameData();

		ImGui::Text("Time Since Start:");	ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "   %s",			clock.GetTimeAsString().c_str());
		ImGui::Text("Frame Count:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "        %llu",	frame_data.frame_count);

		ImGui::Text("Average FPS:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "        %.3f",	frame_data.avg_fps);
		ImGui::Text("Frames Last Second:");	ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), " %u",				frame_data.frames_last_second);
		ImGui::Text("Ms Last Frame:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "      %u",		frame_data.ms_last_frame);
		ImGui::Text("Delta Time:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "         %.3f",	frame_data.dt);

		ImGui::Separator();

		ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Game Time Clock:");

		Hourglass game_clock		= Time::Game::GetClock();
		FrameData game_frame_data	= Time::Game::GetFrameData();

		ImGui::Text("Time Since Start:");	ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "   %s",			game_clock.GetTimeAsString().c_str());
		ImGui::Text("Frame Count:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "        %llu",	game_frame_data.frame_count);

		ImGui::Text("Average FPS:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "        %.3f",	game_frame_data.avg_fps);
		ImGui::Text("Frames Last Second:");	ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), " %u",				game_frame_data.frames_last_second);
		ImGui::Text("Ms Last Frame:");		ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "      %u",		game_frame_data.ms_last_frame);
		ImGui::Text("Delta Time:");			ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "         %.3f",	game_frame_data.dt);
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
		FPS_data[i]	= FPS_data[i + 1];								// The last position is left "empty".
		ms_data[i]	= ms_data[i + 1];								// 
	}																// --------------------

	FPS_data[MAX_HISTOGRAM_SIZE - 1]	= (float)frames;			// Adds to FPS[] the frames per second passed as argument to the last position in it.
	ms_data[MAX_HISTOGRAM_SIZE - 1]		= (float)ms;				// Adds to Ms[] the ms per frame passed as argument to the last position in it.
}

void E_Configuration::PlotFrameDataHistogram()
{
	float average_FPS	= 0.0f;
	float average_ms	= 0.0f;

	for (int i = 0; i < MAX_HISTOGRAM_SIZE; ++i)
	{
		average_FPS += FPS_data[i];
		average_ms	+= ms_data[i];

		peak_FPS	= (peak_FPS < (uint)FPS_data[i])	? (uint)FPS_data[i] : peak_FPS;
		min_FPS		= (min_FPS > (uint)FPS_data[i])		? (uint)FPS_data[i] : min_FPS;
		peak_ms		= (peak_ms < (uint)ms_data[i])		? (uint)ms_data[i] : peak_ms;
		min_ms		= (min_ms > (uint)ms_data[i])		? (uint)ms_data[i] : min_ms;
	}

	average_FPS /= (float)MAX_HISTOGRAM_SIZE;
	average_ms	/= (float)MAX_HISTOGRAM_SIZE;

	char overlay[32];
	sprintf_s(overlay, "Framerate: %.2f", FPS_data[MAX_HISTOGRAM_SIZE - 1]);
	ImGui::PlotHistogram("FPS", FPS_data, IM_ARRAYSIZE(FPS_data), 0, overlay, 0.0f, 120.0f, ImVec2(0, 80));
	ImGui::Text("Average FPS:");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.2f", average_FPS);
	ImGui::Text("Peak FPS:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "   %u", peak_FPS);
	ImGui::Text("Min FPS:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %u", min_FPS);

	sprintf_s(overlay, "ms last frame: %.2f", ms_data[MAX_HISTOGRAM_SIZE - 1]);
	ImGui::PlotHistogram("MS", ms_data, IM_ARRAYSIZE(ms_data), 0, overlay, 0.0f, 40.0f, ImVec2(0, 80));
	ImGui::Text("Average ms: ");	ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "%.2f", average_ms);
	ImGui::Text("Peak ms:");		ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "   %u", peak_ms);
	ImGui::Text("Min ms:");			ImGui::SameLine();	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "    %u", min_ms);
}

void E_Configuration::GenerateFrameCapSlider()
{
	int cap = App->GetFrameCap();

	ImGui::SliderInt("Frame Cap", &cap, 0, 60, "%d", ImGuiSliderFlags_AlwaysClamp);

	App->SetFrameCap(cap);

	if (cap == 0)
	{
		App->frames_are_capped = false;
	}
	else
	{
		App->frames_are_capped = true;														// [ATTENTION] Could be troubling when trying to manage the framecap elsewhere.
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

	uint min_width, min_height, max_width, max_height = 0;
	App->window->GetMinMaxSize(min_width, min_height, max_width, max_height);

	ImGui::SliderInt("Width", &width, min_width, max_width, "%d", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderInt("Height", &height, min_height, max_height, "%d", ImGuiSliderFlags_AlwaysClamp);

	App->window->SetSize(width, height);
}

void E_Configuration::WindowModeFlags()
{
	ImGui::Text("Window Mode:");

	bool maximized			= App->window->IsMaximized();
	bool fullscreen			= App->window->IsFullscreen();
	bool resizable			= App->window->IsResizable();
	bool borderless			= App->window->IsBorderless();
	bool fullscreen_desktop	= App->window->IsFullscreenDesktop();

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

	if (ImGui::Checkbox("Fullscreen Desktop", &fullscreen_desktop))
	{
		App->window->SetFullscreenDesktop(fullscreen_desktop);
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
	float col_dist = 200.0f;
	
	ImGui::Text("Renderer flags: ");

	ImGui::Separator();
	
	// --- OPENGL FLAGS
	bool depth_test					= App->renderer->GetGLFlag(RENDERER_FLAGS::DEPTH_TEST);
	bool cull_face					= App->renderer->GetGLFlag(RENDERER_FLAGS::CULL_FACE);
	bool lighting					= App->renderer->GetGLFlag(RENDERER_FLAGS::LIGHTING);
	bool color_material				= App->renderer->GetGLFlag(RENDERER_FLAGS::COLOR_MATERIAL);
	bool texture_2D					= App->renderer->GetGLFlag(RENDERER_FLAGS::TEXTURE_2D);
	bool alpha_test					= App->renderer->GetGLFlag(RENDERER_FLAGS::ALPHA_TEST);
	bool blend						= App->renderer->GetGLFlag(RENDERER_FLAGS::BLEND);

	// --- SHOW FLAGS
	bool render_world_grid			= App->renderer->GetRenderWorldGrid();
	bool render_world_axis			= App->renderer->GetRenderWorldAxis();
	bool render_wireframes			= App->renderer->GetRenderWireframes();
	bool render_vertex_normals		= App->renderer->GetRenderVertexNormals();
	bool render_face_normals		= App->renderer->GetRenderFaceNormals();
	bool render_bounding_boxes		= App->renderer->GetRenderBoundingBoxes();
	bool render_skeletons			= App->renderer->GetRenderSkeletons();
	bool render_primitive_examples	= App->renderer->GetRenderPrimitiveExamples();
	bool render_others				= false /*App->renderer->GetRenderOthers()*/;

	// --- OPENGL FLAGS
	if (ImGui::Checkbox("Depth Test", &depth_test))									{ App->renderer->SetGLFlag(RENDERER_FLAGS::DEPTH_TEST, depth_test); }			ImGui::SameLine(col_dist);
	if (ImGui::Checkbox("Cull Face", &cull_face))									{ App->renderer->SetGLFlag(RENDERER_FLAGS::CULL_FACE, cull_face); }

	if (ImGui::Checkbox("Lighting", &lighting))										{ App->renderer->SetGLFlag(RENDERER_FLAGS::LIGHTING, lighting); }				ImGui::SameLine(col_dist);
	if (ImGui::Checkbox("Color Material", &color_material))							{ App->renderer->SetGLFlag(RENDERER_FLAGS::COLOR_MATERIAL, color_material); }

	if (ImGui::Checkbox("Texture 2D", &texture_2D))									{ App->renderer->SetGLFlag(RENDERER_FLAGS::TEXTURE_2D, texture_2D); }			ImGui::SameLine(col_dist);
	if (ImGui::Checkbox("Alpha Test", &alpha_test))									{ App->renderer->SetGLFlag(RENDERER_FLAGS::ALPHA_TEST, alpha_test); }

	if (ImGui::Checkbox("Blend", &blend))											{ App->renderer->SetGLFlag(RENDERER_FLAGS::BLEND, blend); }						ImGui::SameLine(col_dist);

	// --- SHOW FLAGS
	if (ImGui::Checkbox("Show World Grid", &render_world_grid))						{ App->renderer->SetRenderWorldGrid(render_world_grid); }

	if (ImGui::Checkbox("Show World Axis", &render_world_axis))						{ App->renderer->SetRenderWorldAxis(render_world_axis); }						ImGui::SameLine(col_dist);
	if (ImGui::Checkbox("Show Wireframes", &render_wireframes))						{ App->renderer->SetRenderWireframes(render_wireframes); }

	if (ImGui::Checkbox("Show Vertex Normals", &render_vertex_normals))				{ App->renderer->SetRenderVertexNormals(render_vertex_normals); }				ImGui::SameLine(col_dist);
	if (ImGui::Checkbox("Show Face Normals", &render_face_normals))					{ App->renderer->SetRenderFaceNormals(render_face_normals); }

	if (ImGui::Checkbox("Show Bounding Boxes", &render_bounding_boxes))				{ App->renderer->SetRenderBoundingBoxes(render_bounding_boxes); }				ImGui::SameLine(col_dist);
	if (ImGui::Checkbox("Show Skeletons", &render_skeletons))						{ App->renderer->SetRenderSkeletons(render_skeletons); }

	if (ImGui::Checkbox("Show Primitive Examples", &render_primitive_examples))		{ App->renderer->SetRenderPrimtiveExamples(render_primitive_examples); }		ImGui::SameLine(col_dist);
	if (ImGui::Checkbox("Show Others (WIP)", &render_others))						{ /*App->renderer->SetRenderOthers();*/ }
}

void E_Configuration::RendererSettings()
{
	float min_line_width		= 0.1f;
	float max_line_width		= 10.0f;

	uint world_grid_size		= App->renderer->GetWorldGridSize();

	Color world_grid_color		= App->renderer->GetWorldGridColor(); 
	Color wireframe_color		= App->renderer->GetWireframeColor();
	Color vertex_normals_color	= App->renderer->GetVertexNormalsColor();
	Color face_normals_color	= App->renderer->GetFaceNormalsColor();

	Color aabb_color			= App->renderer->GetAABBColor();
	Color obb_color				= App->renderer->GetOBBColor();
	Color frustum_color			= App->renderer->GetFrustumColor();
	Color ray_color				= App->renderer->GetRayColor();
	Color bone_color			= App->renderer->GetBoneColor();

	float world_grid_line_width	= App->renderer->GetWorldGridLineWidth();
	float wireframe_line_width	= App->renderer->GetWireframeLineWidth();
	float vertex_normals_width	= App->renderer->GetVertexNormalsWidth();
	float face_normals_width	= App->renderer->GetFaceNormalsWidth();

	float aabb_edge_width		= App->renderer->GetAABBEdgeWidth();
	float obb_edge_width		= App->renderer->GetOBBEdgeWidth();
	float frustum_edge_width	= App->renderer->GetFrustumEdgeWidth();
	float ray_width				= App->renderer->GetRayWidth();
	float bone_width			= App->renderer->GetBoneWidth();
	
	ImGui::Text("Renderer Settings:");

	ImGui::Separator();

	if(ImGui::TreeNodeEx("World Grid"))
	{
		if (ImGui::ColorEdit4("W.G. Color", world_grid_color.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetWorldGridColor(world_grid_color);
		}
		if (ImGui::SliderInt("W.G. Size", (int*)&world_grid_size, 0, 420))
		{
			App->renderer->SetWorldGridSize(world_grid_size);
		}
		if (ImGui::SliderFloat("W.G. L. Width", &world_grid_line_width, min_line_width, max_line_width, "%.3f", 1.0f))
		{
			App->renderer->SetWorldGridLineWidth(world_grid_line_width);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Wireframe"))
	{
		if (ImGui::ColorEdit4("WF. Color", wireframe_color.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetWireframeColor(wireframe_color);
		}
		if (ImGui::SliderFloat("WF. L. Width", &wireframe_line_width, min_line_width, max_line_width, "%.3f", 1.0f))
		{
			App->renderer->SetWireframeLineWidth(wireframe_line_width);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Vertex Normals"))
	{
		if (ImGui::ColorEdit4("V.N. Color", vertex_normals_color.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetVertexNormalsColor(vertex_normals_color);
		}
		if (ImGui::SliderFloat("V.N. L. Width", &vertex_normals_width, min_line_width, max_line_width, "%.3f", 1.0f))
		{
			App->renderer->SetVertexNormalsWidth(vertex_normals_width);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Face Normals"))
	{
		if (ImGui::ColorEdit4("F.N. Color", face_normals_color.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetFaceNormalsColor(face_normals_color);
		}
		if (ImGui::SliderFloat("F.N. L. Width", &face_normals_width, min_line_width, max_line_width, "%.3f", 1.0f))
		{
			App->renderer->SetFaceNormalsWidth(face_normals_width);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("AABB"))
	{
		if (ImGui::ColorEdit4("AABB Color", aabb_color.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetAABBColor(aabb_color);
		}
		if (ImGui::SliderFloat("AABB E. Width", &aabb_edge_width, min_line_width, max_line_width, "%.3f", 1.0f))
		{
			App->renderer->SetAABBEdgeWidth(aabb_edge_width);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("OBB"))
	{
		if (ImGui::ColorEdit4("OBB Color", obb_color.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetOBBColor(obb_color);
		}
		if (ImGui::SliderFloat("OBB E. Width", &obb_edge_width, min_line_width, max_line_width, "%.3f", 1.0f))
		{
			App->renderer->SetOBBEdgeWidth(obb_edge_width);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Frustum"))
	{
		if (ImGui::ColorEdit4("Ftum Color", frustum_color.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetFrustumColor(frustum_color);
		}
		if (ImGui::SliderFloat("Ftum E. Width", &frustum_edge_width, min_line_width, max_line_width, "%.3f", 1.0f))
		{
			App->renderer->SetFrustumEdgeWidth(frustum_edge_width);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Raycast"))
	{
		if (ImGui::ColorEdit4("Ray Color", ray_color.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetRayColor(ray_color);
		}
		if (ImGui::SliderFloat("Ray L. Width", &ray_width, min_line_width, max_line_width, "%.3f", 1.0f))
		{
			App->renderer->SetRayWidth(ray_width);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNodeEx("Skeleton"))
	{
		if (ImGui::ColorEdit4("Bone Color", bone_color.C_Array(), ImGuiColorEditFlags_None))
		{
			App->renderer->SetBoneColor(bone_color);
		}
		if (ImGui::SliderFloat("Bone L. Width", &bone_width, min_line_width, max_line_width, "%.3f", 1.0f))
		{
			App->renderer->SetBoneWidth(bone_width);
		}

		ImGui::TreePop();
	}
}

void E_Configuration::GenerateCameraPositionSlider()
{
	float3 master_camera_position = App->camera->GetMasterCameraPosition();
	if (ImGui::DragFloat3("Position", (float*)&master_camera_position, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
	{
		App->camera->SetMasterCameraPosition(master_camera_position);
	}
}

void E_Configuration::GenerateCameraRotationSlider()
{
	float3 master_camera_rotation = App->camera->GetMasterCameraRotation() * RADTODEG;
	if (ImGui::DragFloat3("Rotation", (float*)&master_camera_rotation, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
	{
		App->camera->SetMasterCameraRotation(master_camera_rotation * DEGTORAD);
	}
}

void E_Configuration::GenerateCameraScaleSlider()
{
	float3 master_camera_scale = App->camera->GetMasterCameraScale();
	if (ImGui::DragFloat3("Scale", (float*)&master_camera_scale, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
	{
		App->camera->SetMasterCameraScale(master_camera_scale);
	}
}

void E_Configuration::GenerateCameraReferenceSlider()
{
	float3 camera_reference = App->camera->GetReference();
	if (ImGui::DragFloat3("Reference", (float*)&camera_reference, 1.0f, 0.0f, 0.0f, "%.3f", NULL))
	{
		App->camera->SetReference(camera_reference);
	}
}

void E_Configuration::GenerateCameraSpeedSliders()
{
	float movement_speed = App->camera->GetMovementSpeed();
	float rotation_speed = App->camera->GetRotationSpeed();
	float zoom_speed = App->camera->GetZoomSpeed();

	ImGui::DragFloat("Movement Speed", &movement_speed, 0.01f, 0.0f, 0.0f, "%.3f", NULL);
	ImGui::DragFloat("Rotation Speed", &rotation_speed, 0.01f, 0.0f, 0.0f, "%.3f", NULL);
	ImGui::DragFloat("Zoom Speed", &zoom_speed, 0.01f, 0.0f, 0.0f, "%.3f", NULL);

	App->camera->SetMovementSpeed(movement_speed);
	App->camera->SetRotationSpeed(rotation_speed);
	App->camera->SetZoomSpeed(zoom_speed);
}

void E_Configuration::GenerateDrawLastRaycastCheckbox()
{
	bool draw_last_raycast = App->camera->DrawLastRaycast();
	if (ImGui::Checkbox("Draw Last Raycast", &draw_last_raycast))
	{
		App->camera->SetDrawLastRaycast(draw_last_raycast);
	}
}

void E_Configuration::AddInputLog(const char* log)
{
	if (log != nullptr)
	{
		char* tmp = _strdup(log);
		
		input_logs.push_back(tmp);

		input_log_scroll_to_bottom = true;
	}
}

void E_Configuration::ReduceInputLog()
{
	if (input_logs.size() > MAX_INPUT_LOG_SIZE)
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
	for (uint i = 0; i < input_logs.size(); ++i)
	{
		ImVec4 text_colour = { 1.0f, 1.0f, 1.0f, 1.0f };									// White is the default colour, but added this to be able to easily change it.					

		if (strstr(input_logs[i], "[KEY]") != nullptr)
		{
			text_colour = { 0.0f, 1.0f, 1.0f, 1.0f };
		}
		
		if (strstr(input_logs[i], "[MOUSE]") != nullptr)
		{
			text_colour = { 1.0f, 0.0f, 1.0f, 1.0f };
		}

		ImGui::PushStyleColor(ImGuiCol_Text, text_colour);
		ImGui::TextUnformatted(input_logs[i]);
		ImGui::PopStyleColor();
	}
}

void E_Configuration::InputLogScrollToBottom()
{
	if (input_log_scroll_to_bottom)
	{
		ImGui::SetScrollHere(1.0f);

		input_log_scroll_to_bottom = false;
	}
}

void E_Configuration::ClearInputLog()
{
	for (uint i = 0; i < input_logs.size(); ++i)
	{
		free(input_logs[i]);
	}

	input_logs.clear();
}

void E_Configuration::GenerateBaseDirectoryText()
{
	ImGui::Text("Base Directory:");
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	ImGui::TextWrapped(App->file_system->GetBaseDirectory());
	ImGui::PopStyleColor();
}

void E_Configuration::GenerateReadDirectoriesText()
{
	ImGui::Text("Read Directories:");
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	ImGui::TextWrapped(App->file_system->GetReadDirectories());
	ImGui::PopStyleColor();
}

void E_Configuration::GenerateWriteDirectoryText()
{
	ImGui::Text("Write Directory:");
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
	ImGui::TextWrapped(App->file_system->GetWriteDirectory());
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