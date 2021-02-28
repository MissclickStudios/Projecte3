#include "Application.h"																						// ATTENTION: Globals.h already included in Module.h
#include "M_Window.h"
#include "M_Renderer3D.h"
#include "M_Editor.h"
#include "M_ResourceManager.h"

#include "M_Input.h"

#include "MemoryManager.h"

#define MAX_KEYS 300
#define MAX_DIR_LENGTH 300

M_Input::M_Input(bool isActive) : Module("Input", isActive)
{
	keyboard = new KeyState[MAX_KEYS];
	memset(keyboard, 0, sizeof(KeyState) * MAX_KEYS);

	maxNumScancodes = (uint)SDL_NUM_SCANCODES;

	mouseX			= 0;
	mouseY			= 0;
	mouseZ			= 0;
	
	mouseMotionX	= 0;
	mouseMotionY	= 0;

	mouseWheelX		= 0;
	mouseWheelY		= 0;

	prevMousePosX	= 0;
	prevMousePosY	= 0;
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
UpdateStatus M_Input::PreUpdate(float dt)
{
	SDL_PumpEvents();

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	for(int i = 0; i < MAX_KEYS; ++i)
	{
		if(keys[i] == 1)
		{
			if (keyboard[i] == KeyState::KEY_IDLE)
			{
				keyboard[i] = KeyState::KEY_DOWN;
			}
			else
			{
				keyboard[i] = KeyState::KEY_REPEAT;
			}
		}
		else
		{
			if (keyboard[i] == KeyState::KEY_REPEAT || keyboard[i] == KeyState::KEY_DOWN)
			{
				keyboard[i] = KeyState::KEY_UP;
			}
			else
			{
				keyboard[i] = KeyState::KEY_IDLE;
			}
		}
	}

	Uint32 buttons = SDL_GetMouseState(&mouseX, &mouseY);

	mouseX /= SCREEN_SIZE;
	mouseY /= SCREEN_SIZE;
	mouseZ = 0;

	for(int i = 0; i < 5; ++i)
	{
		if(buttons & SDL_BUTTON(i))
		{
			if (mouseButtons[i] == KeyState::KEY_IDLE)
			{
				mouseButtons[i] = KeyState::KEY_DOWN;
			}
			else
			{
				mouseButtons[i] = KeyState::KEY_REPEAT;
			}
		}
		else
		{
			if(mouseButtons[i] == KeyState::KEY_REPEAT || mouseButtons[i] == KeyState::KEY_DOWN)
			{
				mouseButtons[i] = KeyState::KEY_UP;
			}
			else
			{
				mouseButtons[i] = KeyState::KEY_IDLE;
			}
		}
	}

	mouseMotionX = mouseMotionY = 0;

	bool quit = false;
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		//Process other modules input
		for (std::vector<Module*>::const_iterator it = ModulesProcessInput.cbegin(); it != ModulesProcessInput.cend(); ++it)
		{
			if((*it)->IsActive())
				(*it)->ProcessInput(event);
		}
		
		switch(event.type)
		{
			case SDL_MOUSEWHEEL:
				mouseWheelX = event.wheel.x;
				mouseWheelY = event.wheel.y;
				mouseZ = event.wheel.y;
			break;

			case SDL_MOUSEMOTION:
				mouseX = event.motion.x / SCREEN_SIZE;
				mouseY = event.motion.y / SCREEN_SIZE;

				mouseMotionX = event.motion.xrel / SCREEN_SIZE;
				mouseMotionY = event.motion.yrel / SCREEN_SIZE;
			break;

			case SDL_QUIT:
				quit = true;
			break;

			case SDL_WINDOWEVENT:
				if (event.window.windowID == SDL_GetWindowID(App->window->GetWindow()))
				{
					if (WindowSizeWasManipulated(event.window.event))
					{
						App->renderer->OnResize();
					}

					if (event.window.event == SDL_WINDOWEVENT_CLOSE)
					{
						App->quit = true;
						return UpdateStatus::STOP;
					}
				}

			break;

			case SDL_DROPFILE:
				if (event.drop.file != nullptr)
				{
					App->resourceManager->ImportFile(event.drop.file);
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

	return UpdateStatus::CONTINUE;
}

UpdateStatus M_Input::Update(float dt)
{
	if (App->input->GetKey(SDL_SCANCODE_F1) == KeyState::KEY_DOWN)
	{
		App->debug = !App->debug;
	}

	return UpdateStatus::CONTINUE;
}

UpdateStatus M_Input::PostUpdate(float dt)
{
	prevMousePosX = GetMouseX();										// Will update the previous mouse position with the current one.
	prevMousePosY = GetMouseY();										// Placed in the PostUpdate() as the Camera controls are called in M_Camera3D's Update().

	return UpdateStatus::CONTINUE;
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
KeyState M_Input::GetKey(int id) const
{
	return keyboard[id];
}

KeyState M_Input::GetMouseButton(int id) const
{
	return mouseButtons[id];
}

uint M_Input::GetMaxNumScancodes() const
{
	return maxNumScancodes;
}

int M_Input::GetMouseX() const
{
	return mouseX;
}

int M_Input::GetMouseY() const
{
	return mouseY;
}

int M_Input::GetMouseZ() const
{
	return mouseZ;
}

int M_Input::GetMouseXMotion() const
{
	return GetMouseX() - prevMousePosX;
}

int M_Input::GetMouseYMotion() const
{
	return GetMouseY() - prevMousePosY;
}

int M_Input::GetMouseXMotionFromSDL() const
{
	return mouseMotionX;
}

int M_Input::GetMouseYMotionFromSDL() const
{
	return mouseMotionY;
}

int M_Input::GetMouseXWheel() const
{
	return mouseWheelX;
}

int M_Input::GetMouseYWheel() const
{
	return mouseWheelY;
}

bool M_Input::WindowSizeWasManipulated(Uint8 windowEvent) const
{
	return (windowEvent == SDL_WINDOWEVENT_RESIZED
			/*|| windowEvent == SDL_WINDOWEVENT_SIZE_CHANGED
			|| windowEvent == SDL_WINDOWEVENT_RESTORED
			|| windowEvent == SDL_WINDOWEVENT_MAXIMIZED
			|| windowEvent == SDL_WINDOWEVENT_MINIMIZED*/);
}

void M_Input::AddModuleToProcessInput(Module* module)
{
	ModulesProcessInput.push_back(module);
}
