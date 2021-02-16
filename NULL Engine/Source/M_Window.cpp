#include "Application.h"																					// ATTENTION: Globals.h already included in Application.h.

#include "M_Window.h"

M_Window::M_Window(bool is_active) : Module("Window", is_active)
{
	window					= nullptr;
	screen_surface			= nullptr;

	screen_width			= 0;
	screen_height			= 0;

	is_maximized			= WIN_MAXIMIZED;
	is_fullscreen			= WIN_FULLSCREEN;
	is_resizable			= WIN_RESIZABLE;
	is_borderless			= WIN_BORDERLESS;
	is_fullscreen_desktop	= WIN_FULLSCREEN_DESKTOP;
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
		screen_width	= SCREEN_WIDTH * SCREEN_SIZE;
		screen_height	= SCREEN_HEIGHT * SCREEN_SIZE;
		Uint32 flags	= SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		//Use OpenGL 2.1
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		
		if (WIN_MAXIMIZED == true)
		{
			flags |= SDL_WINDOW_MAXIMIZED;
		}
		
		if(WIN_FULLSCREEN == true)
		{
			flags |= SDL_WINDOW_FULLSCREEN;
		}

		if(WIN_RESIZABLE == true)
		{
			flags |= SDL_WINDOW_RESIZABLE;
		}

		if(WIN_BORDERLESS == true)
		{
			flags |= SDL_WINDOW_BORDERLESS;
		}

		if(WIN_FULLSCREEN_DESKTOP == true)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

		window = SDL_CreateWindow(TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, flags);

		if(window == nullptr)
		{
			LOG("Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		else
		{
			//Get window surface
			screen_surface = SDL_GetWindowSurface(window);
			SDL_GetWindowSize(window, (int*)&screen_width, (int*)&screen_height);												// Dirty fix to generate the framebuffer correctly.
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
	bool ret = true;

	return ret;
}

bool M_Window::SaveConfiguration(ParsonNode& root) const
{
	bool ret = true;

	return ret;
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
	return screen_width;
}

uint M_Window::GetHeight() const
{
	return screen_height;
}

void M_Window::GetMinMaxSize(uint& min_width, uint& min_height, uint& max_width, uint& max_height) const
{
	min_width	= SCREEN_MIN_WIDTH;
	min_height	= SCREEN_MIN_HEIGHT;
	max_width	= SCREEN_MAX_WIDTH;
	max_height	= SCREEN_MAX_HEIGHT;

	SDL_DisplayMode display_mode;
	if (SDL_GetDesktopDisplayMode(0, &display_mode) != 0)													// SDL_GetDesktopDisplayMode() returns 0 on success and -1 on failure.
	{
		LOG("[ERROR] SDL_GetDesktopDisplayMode() failed! SDL_Error: %s", SDL_GetError());
	}
	else
	{
		max_width	= display_mode.w;
		max_height	= display_mode.h;
	}
}

float M_Window::GetBrightness() const
{
	return SDL_GetWindowBrightness(window);
}

int M_Window::GetRefreshRate() const
{
	SDL_DisplayMode display_mode;
	if (SDL_GetDesktopDisplayMode(0, &display_mode) != 0)
	{
		LOG("[ERROR] SDL_GetDesktopDisplayMode() failed! SDL_Error: %s", SDL_GetError());
	}
	else
	{
		return display_mode.refresh_rate;
	}

	return 0;
}

void M_Window::SetWidth(uint width)
{
	screen_width = width;
	SDL_SetWindowSize(window, screen_width, screen_height);
}

void M_Window::SetHeight(uint height)
{
	screen_height = height;
	SDL_SetWindowSize(window, screen_width, screen_height);
}

void M_Window::SetSize(uint width, uint height)
{
	screen_width = width;
	screen_height = height;

	SDL_SetWindowSize(window, screen_width, screen_height);
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
	return is_maximized;
}

bool M_Window::IsFullscreen() const
{
	return is_fullscreen;
}

bool M_Window::IsResizable() const
{
	return is_resizable;
}

bool M_Window::IsBorderless() const
{
	return is_borderless;
}

bool M_Window::IsFullscreenDesktop() const
{
	return is_fullscreen_desktop;
}

void M_Window::SetMaximized(bool set_to)
{
	if (set_to != is_maximized)
	{
		is_maximized = set_to;
		
		if (set_to)
		{
			SDL_MaximizeWindow(window);

			SetFullscreen(!set_to);																// If window is maximized it cannot be in fullscreen or fullscreen desktop mode.
			SetFullscreenDesktop(!set_to);														// ---------

			SDL_GetWindowSize(window, (int*)&screen_width, (int*)&screen_height);

			LOG("[STATUS] MAXIMIZED SCREEN SIZE: %u x %u", screen_width, screen_height);
			LOG("[STATUS] Window has been Maximized!");
		}
		else
		{	
			SDL_RestoreWindow(window);
			
			SDL_GetWindowSize(window, (int*)&screen_width, (int*)&screen_height);
			
			LOG("[STATUS] Window has been resized to { %u, %u }", SCREEN_WIDTH, SCREEN_HEIGHT);
		}
	}
}

void M_Window::SetFullscreen(bool set_to)
{
	if (set_to != is_fullscreen)
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

void M_Window::SetResizable(bool set_to)
{
	// Cannot be changed while the application is running, but the change itself can be saved.
	is_resizable = set_to;
	
	/*if (set_to)
	{
		SDL_SetWindowResizable(window, SDL_TRUE);
	}
	else
	{
		SDL_SetWindowResizable(window, SDL_FALSE);
	}*/
}

void M_Window::SetBorderless(bool set_to)
{
	if (set_to != is_borderless && !is_fullscreen && !is_fullscreen_desktop)
	{
		is_borderless = set_to;																						
		SDL_SetWindowBordered(window, (SDL_bool)!is_borderless);													// SDL_SetWindowBodered makes the window borderless on SDL_FALSE.
	}
}

void M_Window::SetFullscreenDesktop(bool set_to)
{	
	if (set_to != is_fullscreen_desktop)
	{
		if (set_to)
		{
			int result = SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

			if (result != 0)
			{
				LOG("[ERROR] SDL_SetWindowFullscreen() failed! SDL_Error: %s", SDL_GetError());						// SDL_SetWindowFullscreen() returns 0 on success and -1 on failure.
			}
			else
			{
				is_fullscreen_desktop = set_to;
				is_maximized = !set_to;

				SDL_GetWindowSize(window, (int*)&screen_width, (int*)&screen_height);

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
				is_fullscreen_desktop = set_to;
				is_fullscreen = set_to;

				SDL_GetWindowSize(window, (int*)&screen_width, (int*)&screen_height);

				LOG("[STATUS] Window has been resized to { %u, %u }", SCREEN_WIDTH, SCREEN_HEIGHT);
			}
		}
	}
}