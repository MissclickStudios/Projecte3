#include "Application.h"																						// ATTENTION: Globals.h already included in Module.h
#include "M_Window.h"
#include "M_Renderer3D.h"
#include "M_Editor.h"
#include "M_ResourceManager.h"

#include "M_Input.h"

#include "MemoryManager.h"

#define MAX_KEYS 300
#define MAX_DIR_LENGTH 300

M_Input::M_Input(bool is_active) : Module("Input", is_active)
{
	keyboard = new KEY_STATE[MAX_KEYS];
	memset(keyboard, 0, sizeof(KEY_STATE) * MAX_KEYS);

	max_num_scancodes = (uint)SDL_NUM_SCANCODES;

	mouse_x			= 0;
	mouse_y			= 0;
	mouse_z			= 0;
	
	mouse_x_motion	= 0;
	mouse_y_motion	= 0;

	mouse_x_wheel	= 0;
	mouse_y_wheel	= 0;

	prev_x_mouse_pos = 0;
	prev_y_mouse_pos = 0;
}

// Destructor
M_Input::~M_Input()
{
	delete[] keyboard;
}

// Called before render is available
bool M_Input::Init(ParsonNode& config)
{
	LOG("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	if(SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	return ret;
}

// Called every draw update
UPDATE_STATUS M_Input::PreUpdate(float dt)
{
	SDL_PumpEvents();

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	for(int i = 0; i < MAX_KEYS; ++i)
	{
		if(keys[i] == 1)
		{
			if (keyboard[i] == KEY_STATE::KEY_IDLE)
			{
				keyboard[i] = KEY_STATE::KEY_DOWN;
				App->editor->AddInputLog(i, (uint)KEY_STATE::KEY_DOWN);
			}
			else
			{
				keyboard[i] = KEY_STATE::KEY_REPEAT;
				App->editor->AddInputLog(i, (uint)KEY_STATE::KEY_REPEAT);
			}
		}
		else
		{
			if (keyboard[i] == KEY_STATE::KEY_REPEAT || keyboard[i] == KEY_STATE::KEY_DOWN)
			{
				keyboard[i] = KEY_STATE::KEY_UP;
				App->editor->AddInputLog(i, (uint)KEY_STATE::KEY_UP);
			}
			else
			{
				keyboard[i] = KEY_STATE::KEY_IDLE;
			}
		}
	}

	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

	mouse_x /= SCREEN_SIZE;
	mouse_y /= SCREEN_SIZE;
	mouse_z = 0;

	for(int i = 0; i < 5; ++i)
	{
		if(buttons & SDL_BUTTON(i))
		{
			if (mouse_buttons[i] == KEY_STATE::KEY_IDLE)
			{
				mouse_buttons[i] = KEY_STATE::KEY_DOWN;
				App->editor->AddInputLog(max_num_scancodes + i, (uint)KEY_STATE::KEY_DOWN);
			}
			else
			{
				mouse_buttons[i] = KEY_STATE::KEY_REPEAT;
				App->editor->AddInputLog(max_num_scancodes + i, (uint)KEY_STATE::KEY_REPEAT);
			}
		}
		else
		{
			if(mouse_buttons[i] == KEY_STATE::KEY_REPEAT || mouse_buttons[i] == KEY_STATE::KEY_DOWN)
			{
				mouse_buttons[i] = KEY_STATE::KEY_UP;
				App->editor->AddInputLog(max_num_scancodes + i, (uint)KEY_STATE::KEY_UP);
			}
			else
			{
				mouse_buttons[i] = KEY_STATE::KEY_IDLE;
			}
		}
	}

	mouse_x_motion = mouse_y_motion = 0;

	bool quit = false;
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		App->editor->GetEvent(&event);
		
		switch(event.type)
		{
			case SDL_MOUSEWHEEL:
				mouse_x_wheel = event.wheel.x;
				mouse_y_wheel = event.wheel.y;
				mouse_z = event.wheel.y;
			break;

			case SDL_MOUSEMOTION:
				mouse_x = event.motion.x / SCREEN_SIZE;
				mouse_y = event.motion.y / SCREEN_SIZE;

				mouse_x_motion = event.motion.xrel / SCREEN_SIZE;
				mouse_y_motion = event.motion.yrel / SCREEN_SIZE;
			break;

			case SDL_QUIT:
				quit = true;
			break;

			case SDL_WINDOWEVENT:
				if (event.window.windowID == SDL_GetWindowID(App->window->GetWindow()))
				{
					if (event.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						App->renderer->OnResize();
					}

					if (event.window.event == SDL_WINDOWEVENT_CLOSE)
					{
						App->quit = true;
						return UPDATE_STATUS::STOP;
					}
				}

			break;

			case SDL_DROPFILE:
				if (event.drop.file != nullptr)
				{
					App->resource_manager->ImportFile(event.drop.file);
					//Importer::ImportFile(event.drop.file);
				}
				else
				{
					LOG("[ERROR] Input: String from SDL_DROPFILE Event was nullptr!");
				}

				SDL_free(event.drop.file);

			break;
		}
	}

	return UPDATE_STATUS::CONTINUE;
}

UPDATE_STATUS M_Input::Update(float dt)
{
	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_STATE::KEY_DOWN)
	{
		App->debug = !App->debug;
	}

	return UPDATE_STATUS::CONTINUE;
}

UPDATE_STATUS M_Input::PostUpdate(float dt)
{
	prev_x_mouse_pos = GetMouseX();										// Will update the previous mouse position with the current one.
	prev_y_mouse_pos = GetMouseY();										// Placed in the PostUpdate() as the Camera controls are called in M_Camera3D's Update().

	return UPDATE_STATUS::CONTINUE;
}

// Called before quitting
bool M_Input::CleanUp()
{
	LOG("Quitting SDL input event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	return true;
}

bool M_Input::LoadConfiguration(ParsonNode& root)
{
	bool ret = true;

	return ret;
}

bool M_Input::SaveConfiguration(ParsonNode& root) const
{
	bool ret = true;

	return ret;
}

// --------- INPUT METHODS ---------
KEY_STATE M_Input::GetKey(int id) const
{
	return keyboard[id];
}

KEY_STATE M_Input::GetMouseButton(int id) const
{
	return mouse_buttons[id];
}

uint M_Input::GetMaxNumScancodes() const
{
	return max_num_scancodes;
}

int M_Input::GetMouseX() const
{
	return mouse_x;
}

int M_Input::GetMouseY() const
{
	return mouse_y;
}

int M_Input::GetMouseZ() const
{
	return mouse_z;
}

int M_Input::GetMouseXMotion() const
{
	return GetMouseX() - prev_x_mouse_pos;
}

int M_Input::GetMouseYMotion() const
{
	return GetMouseY() - prev_y_mouse_pos;
}

int M_Input::GetMouseXMotionFromSDL() const
{
	return mouse_x_motion;
}

int M_Input::GetMouseYMotionFromSDL() const
{
	return mouse_y_motion;
}

int M_Input::GetMouseXWheel() const
{
	return mouse_x_wheel;
}

int M_Input::GetMouseYWheel() const
{
	return mouse_y_wheel;
}