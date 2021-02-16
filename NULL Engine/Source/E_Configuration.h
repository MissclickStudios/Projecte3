#ifndef __E_CONFIGURATION_H__
#define __E_CONFIGURATION_H__

#include "EditorPanel.h"

struct HardwareInfo;

#define MAX_HISTOGRAM_SIZE 100

class E_Configuration : public EditorPanel
{
public:
	E_Configuration();
	~E_Configuration();

	bool Draw		(ImGuiIO& io) override;
	bool CleanUp	() override;

public:
	void UpdateFrameData				(int frames, int ms);
	void AddInputLog					(const char* log);

private:
	bool ConfigurationMenuBar			();								// Will generate the elements of the configuration window's toolbar.
	
	bool ApplicationMenu				();								// Menu for the Application module.
	bool WindowMenu						();								// Menu for the Window module.
	bool RendererMenu					();								// Menu for the Renderer3D module.
	bool CameraMenu						();								// Menu for the Camera3D module.
	bool InputMenu						();								// Menu for the Input module.
	bool FileSystemMenu					();								// Menu for the FileSystem module.
	bool TexturesMenu					();								// 
	bool AudioMenu						();								// 
	bool PhysicsMenu					();								// 
	bool SystemInfoMenu					();								// Menu that will display the hardware state of the system that is running the application.
	bool TimeManagementMenu				();								//
	bool ImGuiEditorsMenu				();								// Menu that will allow to dynamically change the style of the ImGui UI and export the current style.

	// ------- MENU METHODS -------
	void PlotFrameDataHistogram			();								// Application menu: Will plot the frame data histogram according to the current frame data values.
	void GenerateFrameCapSlider			();								// Application menu: Will generate a slider that will modify the frame cap value. From 0 (no cap) to 60.

	void GenerateBrightnessSlider		();								// Window menu: Will generate a slider that will modify the brightness of the whole window.
	void GenerateSizeSliders			();								// Window menu: Will generate 2 sliders. One will modify the width of the window and the other will modify the size.
	void WindowModeFlags				();								// Window menu: Will generate the checkboxes that will modify the window mode. Ex: Maximized, Fullscreen...

	void VsyncMode						();								// Renderer menu: Will generate a checkbox that will enable/disable the Vsync mode.
	void RendererFlags					();								// Renderer menu: Will generate the checkboxes that will modify the renderer flags.
	void RendererSettings				();

	void GenerateCameraPositionSlider	();								// Camera menu: Will generate a slider with 3 inputs that will modify the camera's position (XYZ).
	void GenerateCameraRotationSlider	();								// Camera menu: Will generate a slider with 3 inputs that will modify the camera's rotation (XYZ).
	void GenerateCameraScaleSlider		();								// Camera menu: Will generate a slider with 3 inputs that will modify the camera's scale (XYZ).
	void GenerateCameraReferenceSlider	();								// Camera menu: Will generate a slider with 3 inputs that will modify the camera's reference (XYZ).
	void GenerateCameraSpeedSliders		();								// Camera menu: Will generate 3 sliders that will modify the camera's movement, rotation and zoom speed.
	void GenerateDrawLastRaycastCheckbox();

	void MouseInputData					();								// Input menu: Will generate 3 pairs of two texts, one of the pair with the type of data and the second with its val.
	void InputLogOutput					();								// Input menu: Will generate the text of the Log. The logs are stored in the input_logs std::vector.
	void ReduceInputLog					();								// Input menu: Will reduce the size of the input_logs vector down to a manageable size, memory and framerate wise.
	void InputLogScrollToBottom			();								// Input menu: Will scroll down to the bottommost part of the Log text whenever it is called.
	void ClearInputLog					();								// Input menu: Will delete every log and empty the input_logs std::vector.

	void GenerateBaseDirectoryText		();								// FileSystem menu: Will generate the texts that will display the Base Directory path.
	void GenerateReadDirectoriesText	();								// FileSystem menu: Will generate the texts that will display the Read Directory path.
	void GenerateWriteDirectoryText		();								// FileSystem menu: Will generate the texts that will display the Write Directory path.

	void SDLInfo						(HardwareInfo* hw_info);		// Hardware menu: Will generate the texts that will display information related with the SDL library.
	void OpenGLInfo						(HardwareInfo* hw_info);		// Hardware menu: Will generate the texts that will display information related with the OpenGL library.
	void DevILInfo						(HardwareInfo* hw_info);		// Hardware menu: Will generate the texts that will display information related with the DevIL library.
	void CPUInfo						(HardwareInfo* hw_info);		// Hardware menu: Will generate the texts that will display information related with the CPU.
	void GPUInfo						(HardwareInfo* hw_info);		// Hardware menu: Will generate the texts that will display information related with the GPU.

private:
	float FPS_data[MAX_HISTOGRAM_SIZE];
	float ms_data[MAX_HISTOGRAM_SIZE];

	uint peak_FPS;
	uint min_FPS;
	uint peak_ms;
	uint min_ms;

	std::vector<char*> input_logs;
	bool input_log_scroll_to_bottom;
};

#endif // !_E_CONFIGURATION_H__