#include "Application.h"
#include "ConfigurationSettings.h"
#include "Log.h"
#include "JSONParser.h"
#include "M_Window.h"

#include "MemoryManager.h"

M_Window::M_Window(bool isActive) : Module("Window", isActive)
{
	window				= nullptr;
	screenSurface		= nullptr;

	screenWidth			= SCREEN_WIDTH;
	screenHeight		= SCREEN_HEIGHT;

	isMaximized			= WIN_MAXIMIZED;
	isFullscreen		= WIN_FULLSCREEN;
	isResizable			= WIN_RESIZABLE;
	isBorderless		= WIN_BORDERLESS;
	isFullscreenDesktop	= WIN_FULLSCREEN_DESKTOP;
}

// Destructor
M_Window::~M_Window()
{
}

// Called before render is available
bool M_Window::Init(ParsonNode& config)
{
	LOG("Init SDL window & surface");
	bool ret = true;

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LOG("SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		//Create window

		screenWidth = config.GetInteger("screenWidth");
		screenHeight = config.GetInteger("screenHeight");

		if (screenWidth <= 0)
			screenWidth = 1280;
		if (screenHeight <= 0)
			screenHeight = 720;

		isMaximized = config.GetBool("isMaximized");
		isFullscreen = config.GetBool("isFullscreen");
		isResizable = config.GetBool("isResizable");
		isBorderless = config.GetBool("isBorderless");
		isFullscreenDesktop = config.GetBool("isFullscreenDesktop");

		Uint32 flags	= SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
		//Use Stencil Buffer

		//SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

		//Use OpenGL 2.1
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		
		if (isMaximized)
			flags |= SDL_WINDOW_MAXIMIZED;
		
		if(isFullscreen)
			flags |= SDL_WINDOW_FULLSCREEN;

		if(isResizable)
			flags |= SDL_WINDOW_RESIZABLE;

		if(isBorderless)
			flags |= SDL_WINDOW_BORDERLESS;

		if(isFullscreenDesktop)
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screenWidth, screenHeight, flags);

		if(window == nullptr)
		{
			LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		else
		{
			//Get window surface
			screenSurface = SDL_GetWindowSurface(window);
			SDL_GetWindowSize(window, (int*)&screenWidth, (int*)&screenHeight);												// Dirty fix to generate the framebuffer correctly.
		}
	}

	return ret;
}

// Called before quitting
bool M_Window::CleanUp()
{
	LOG("Destroying SDL window and quitting all SDL systems");

	//Destroy window
	if(window != nullptr)
	{
		SDL_DestroyWindow(window);
	}

	//Quit SDL subsystems
	SDL_Quit();
	return true;
}

bool M_Window::LoadConfiguration(ParsonNode& root)
{

	return true;
}

bool M_Window::SaveConfiguration(ParsonNode& root) const
{
	root.SetInteger("screenWidth", screenWidth);
	root.SetInteger("screenHeight", screenHeight);

	root.SetBool("isMaximized",isMaximized);
	root.SetBool("isFullscreen",isFullscreen);
	root.SetBool("isResizable",isResizable);
	root.SetBool("isBorderless",isBorderless);
	root.SetBool("isFullscreenDesktop",isFullscreenDesktop);

	return true;
}

// -------- WINDOW METHODS --------
SDL_Window* M_Window::GetWindow() const
{
	return window;
}

void M_Window::SetTitle(const char* title)
{
	SDL_SetWindowTitle(window, title);
}

const char* M_Window::GetIcon() const
{
	return nullptr;
}

void M_Window::SetIcon(const char* file)
{

}

void M_Window::SetDefaultIcon()
{

}

uint M_Window::GetWidth() const
{
	return screenWidth;
}

uint M_Window::GetHeight() const
{
	return screenHeight;
}

void M_Window::GetMinMaxSize(uint& minWidth, uint& minHeight, uint& maxWidth, uint& maxHeight) const
{
	minWidth	= SCREEN_MIN_WIDTH;
	minHeight	= SCREEN_MIN_HEIGHT;
	maxWidth	= SCREEN_MAX_WIDTH;
	maxHeight	= SCREEN_MAX_HEIGHT;

	SDL_DisplayMode displayMode;
	if (SDL_GetDesktopDisplayMode(0, &displayMode) != 0)													// SDL_GetDesktopDisplayMode() returns 0 on success and -1 on failure.
	{
		LOG("[ERROR] SDL_GetDesktopDisplayMode() failed! SDL_Error: %s", SDL_GetError());
	}
	else
	{
		maxWidth	= displayMode.w;
		maxHeight	= displayMode.h;
	}
}

float M_Window::GetBrightness() const
{
	return SDL_GetWindowBrightness(window);
}

int M_Window::GetRefreshRate() const
{
	SDL_DisplayMode displayMode;
	if (SDL_GetDesktopDisplayMode(0, &displayMode) != 0)
	{
		LOG("[ERROR] SDL_GetDesktopDisplayMode() failed! SDL_Error: %s", SDL_GetError());
	}
	else
	{
		return displayMode.refresh_rate;
	}

	return 0;
}

void M_Window::SetWidth(uint width)
{
	screenWidth = width;
	SDL_SetWindowSize(window, screenWidth, screenHeight);
}

void M_Window::SetHeight(uint height)
{
	screenHeight = height;
	SDL_SetWindowSize(window, screenWidth, screenHeight);
}

void M_Window::SetSize(uint width, uint height)
{
	screenWidth = width;
	screenHeight = height;

	SDL_SetWindowSize(window, screenWidth, screenHeight);
}

void M_Window::SetBrightness(float brightness)
{
	int result = SDL_SetWindowBrightness(window, brightness);												// SDL_SetWindowBrightness() returns 0 on success and -1 on failure.

	if (result != 0)
	{
		LOG("[ERROR] SDL_SetWindowBrightness() failed! SDL_Error: %s", SDL_GetError());
	}
}

bool M_Window::IsMaximized() const
{
	return isMaximized;
}

bool M_Window::IsFullscreen() const
{
	return isFullscreen;
}

bool M_Window::IsResizable() const
{
	return isResizable;
}

bool M_Window::IsBorderless() const
{
	return isBorderless;
}

bool M_Window::IsFullscreenDesktop() const
{
	return isFullscreenDesktop;
}

void M_Window::SetMaximized(bool setTo)
{
	if (setTo != isMaximized)
	{
		isMaximized = setTo;
		
		if (setTo)
		{
			SDL_MaximizeWindow(window);

			SetFullscreen(!setTo);																// If window is maximized it cannot be in fullscreen or fullscreen desktop mode.
			SetFullscreenDesktop(!setTo);														// ---------

			LOG("[STATUS] Window has been Maximized!");
		}
		else
		{	
			SDL_RestoreWindow(window);

			LOG("[STATUS] Window has been Unmaximized!");
		}

		RecalculateWindowSize();

		LOG("[STATUS] New Window Size { %u, %u }", screenWidth, screenHeight);
	}
}

void M_Window::SetFullscreen(bool setTo)
{
	if (setTo != isFullscreen)
	{
		LOG("[STATUS] Window: Fullscreen mode is currently disabled!");
		
		//if (set_to)
		//{
		//	int result = SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);									// SDL_SetWindowFullscreen() returns 0 on success and -1 on failure.

		//	if (result != 0)
		//	{
		//		LOG("[ERROR] SDL_SetWindowFullscreen() failed! SDL_Error: %s", SDL_GetError());
		//	}
		//	else
		//	{
		//		is_fullscreen = set_to;
		//		is_maximized = !set_to;

		//		SDL_GetWindowSize(window, (int*)&screen_width, (int*)&screen_height);

		//		LOG("[STATUS] Window has been set to Fullscreen mode!");
		//	}
		//}
		//else
		//{
		//	int result = SDL_SetWindowFullscreen(window, 0);														// The 0 "flag" sets the window to windowed mode.

		//	if (result != 0)
		//	{
		//		LOG("[ERROR] SDL_SetWindowFullscreen() failed! SDL_Error: %s", SDL_GetError());
		//	}
		//	else
		//	{
		//		is_fullscreen = set_to;
		//		is_fullscreen_desktop = set_to;

		//		SDL_GetWindowSize(window, (int*)&screen_width, (int*)&screen_height);

		//		LOG("[STATUS] Window has been resized to { %u, %u }", SCREEN_WIDTH, SCREEN_HEIGHT);
		//	}
		//}
	}
}

void M_Window::SetResizable(bool setTo)
{
	// Cannot be changed while the application is running, but the change itself can be saved.
	isResizable = setTo;
	
	/*if (set_to)
	{
		SDL_SetWindowResizable(window, SDL_TRUE);
	}
	else
	{
		SDL_SetWindowResizable(window, SDL_FALSE);
	}*/
}

void M_Window::SetBorderless(bool setTo)
{
	if (setTo != isBorderless && !isFullscreen && !isFullscreenDesktop)
	{
		isBorderless = setTo;																						
		SDL_SetWindowBordered(window, (SDL_bool)!isBorderless);													// SDL_SetWindowBodered makes the window borderless on SDL_FALSE.
	}
}

void M_Window::SetFullscreenDesktop(bool setTo)
{	
	if (setTo != isFullscreenDesktop)
	{
		if (setTo)
		{
			int result = SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

			if (result != 0)
			{
				LOG("[ERROR] SDL_SetWindowFullscreen() failed! SDL_Error: %s", SDL_GetError());						// SDL_SetWindowFullscreen() returns 0 on success and -1 on failure.
			}
			else
			{
				isFullscreenDesktop = setTo;
				isMaximized = !setTo;

				RecalculateWindowSize();

				LOG("[STATUS] Window has been set to Fullscreen Desktop mode!");
			}
		}
		else
		{
			int result = SDL_SetWindowFullscreen(window, 0);

			if (result != 0)
			{
				LOG("[ERROR] SDL_SetWindowFullscreen() failed! SDL_Error: %s", SDL_GetError());						// The 0 "flag" sets the window to windowed mode.
			}
			else
			{
				isFullscreenDesktop = setTo;
				isFullscreen = setTo;

				RecalculateWindowSize();

				LOG("[STATUS] Window has been resized to { %u, %u }", SCREEN_WIDTH, SCREEN_HEIGHT);
			}
		}
	}
}

void M_Window::RecalculateWindowSize()
{
	SDL_GetWindowSize(window, (int*)&screenWidth, (int*)&screenHeight);
}