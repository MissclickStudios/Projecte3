#include "Application.h"																						// ATTENTION: Globals.h already included in Module.h
#include "M_Window.h"
#include "M_Renderer3D.h"
#include "M_Editor.h"
#include "M_ResourceManager.h"

#include "M_Input.h"

#include "MemoryManager.h"
#include "Profiler.h"

#define MAX_KEYS 300
#define MAX_DIR_LENGTH 300

M_Input::M_Input(bool isActive) : Module("Input", isActive)
{
	keyboard = new KeyState[MAX_KEYS];
	memset(keyboard, 0, sizeof(KeyState) * MAX_KEYS);

	maxNumScancodes = (uint)SDL_NUM_SCANCODES;

	mouseX = 0;
	mouseY = 0;
	mouseZ = 0;

	mouseMotionX = 0;
	mouseMotionY = 0;

	mouseWheelX = 0;
	mouseWheelY = 0;

	prevMousePosX = 0;
	prevMousePosY = 0;

	gameController.buttons = new ButtonState[NUM_CONTROLLER_BUTTONS];
	memset(gameController.buttons, 0, sizeof(ButtonState) * NUM_CONTROLLER_BUTTONS);		// 0 = BUTTON_IDLE

	gameController.triggers = new ButtonState[NUM_CONTROLLER_TRIGGERS];					// Only 2 triggers are supported for a given game controller.
	memset(gameController.triggers, 0, sizeof(ButtonState) * NUM_CONTROLLER_TRIGGERS);	// 0 = BUTTON_IDLE

	gameController.axis = new AxisState[NUM_CONTROLLER_AXIS];								// Only 4 axis are supported for a given game controller.
	memset(gameController.axis, 0, sizeof(AxisState) * NUM_CONTROLLER_AXIS);				// 0 = AXIS_IDLE

	gameController.id = nullptr;
	gameController.index = CONTROLLER_INDEX;
	gameController.max_axis_input_threshold = 0.5f;
	gameController.min_axis_input_threshold = 0.2f;
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

	if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)											//Will initialize the game controller subsystem as well as the joystick subsystem.
	{
		LOG("SDL_GAMECONTROLLER could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	SDL_GameControllerEventState(SDL_ENABLE);													//Enable/Disable events dealing with Game Controllers. SDL_QUERY and SDL_ENABLE/IGNORE.

	if (SDL_GameControllerEventState(SDL_QUERY) != SDL_ENABLE)									//Checking the state of the game controller event. QUERY will return 1 on ENABLE and 0 on IGNORE.
	{
		LOG("The Controller Event State could not be enabled! SDL_Error: %s\n", SDL_GetError());
		ret = false;		// CARE. CAN CLOSE THE APPLICATION.
	}

	if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	return ret;
}

// Called every draw update
UpdateStatus M_Input::PreUpdate(float dt)
{
	OPTICK_CATEGORY("M_Input PreUpdate", Optick::Category::Module)

	OPTICK_CATEGORY("M_Input PreUpdate: Mouse", Optick::Category::Input)
	SDL_PumpEvents();

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	for (int i = 0; i < MAX_KEYS; ++i)
	{
		if (keys[i] == 1)
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

	OPTICK_CATEGORY("M_Input PreUpdate: Mouse", Optick::Category::Input)

	Uint32 buttons = SDL_GetMouseState(&mouseX, &mouseY);

	mouseX /= SCREEN_SIZE;
	mouseY /= SCREEN_SIZE;
	mouseZ = 0;

	for (int i = 0; i < 5; ++i)
	{
		if (buttons & SDL_BUTTON(i))
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
			if (mouseButtons[i] == KeyState::KEY_REPEAT || mouseButtons[i] == KeyState::KEY_DOWN)
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

	OPTICK_CATEGORY("M_Input PreUpdate: Events", Optick::Category::Input)
	bool quit = false;
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		//Process other modules input
		for (std::vector<Module*>::const_iterator it = ModulesProcessInput.cbegin(); it != ModulesProcessInput.cend(); ++it)
		{
			if ((*it)->IsActive())
				(*it)->ProcessInput(event);
		}

		switch (event.type)
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
				//App->resourceManager->ImportFile(event.drop.file);
				App->resourceManager->DragAndDrop(event.drop.file);
				//Importer::ImportFile(event.drop.file);
			}
			else
			{
				LOG("[ERROR] Input: String from SDL_DROPFILE Event was nullptr!");
			}

			SDL_free(event.drop.file);

			break;

		case SDL_CONTROLLERDEVICEADDED:
			if (gameController.id == nullptr)
			{
				if (SDL_NumJoysticks() > 0)
				{
					if (SDL_IsGameController(CONTROLLER_INDEX))
					{
						gameController.id = SDL_GameControllerOpen(CONTROLLER_INDEX);
					}
				}
				else
				{
					LOG("No joysticks currently attached to the system. SDL_Error: %s\n", SDL_GetError());
				}
			}
		}
	}

	OPTICK_CATEGORY("M_Input PreUpdate: Controller", Optick::Category::Input)

	if (SDL_GameControllerGetAttached(gameController.id))														// Returns true if the given game controller is open and currently connected.
	{
		// --- GAME CONTROLLER BUTTON STATES ---
		for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; ++i)														// Will iterate through all the buttons available. Same as the keyboard key loop.
		{
			if (SDL_GameControllerGetButton(gameController.id, SDL_GameControllerButton(i)) == SDL_PRESSED)	// Gets the current state of a button in a controller. 1 = PRESSED / 0 = RELEASED.
			{
				if (gameController.buttons[i] == ButtonState::BUTTON_IDLE)
				{
					gameController.buttons[i] = ButtonState::BUTTON_DOWN;
				}
				else
				{
					gameController.buttons[i] = ButtonState::BUTTON_REPEAT;
				}
			}
			else
			{
				if (gameController.buttons[i] == ButtonState::BUTTON_DOWN || gameController.buttons[i] == ButtonState::BUTTON_REPEAT)
				{
					gameController.buttons[i] = ButtonState::BUTTON_UP;
				}
				else
				{
					gameController.buttons[i] = ButtonState::BUTTON_IDLE;
				}
			}
		}

		// --- GAME CONTROLLER TRIGGER STATES ---
		for (int i = 0; i < NUM_CONTROLLER_TRIGGERS; ++i)
		{
			int trigger_value = SDL_GameControllerGetAxis(gameController.id, SDL_GameControllerAxis(TRIGGER_INDEX + i));
			//LOG("CURRENT AXIS VALUE IS: %d", trigger_value);

			if (trigger_value > gameController.max_axis_input_threshold* MAX_AXIS)
			{
				if (gameController.triggers[i] == ButtonState::BUTTON_IDLE)
				{
					gameController.triggers[i] = ButtonState::BUTTON_DOWN;
				}
				else
				{
					gameController.triggers[i] = ButtonState::BUTTON_REPEAT;
				}
			}
			else
			{
				if (gameController.triggers[i] == ButtonState::BUTTON_DOWN || gameController.triggers[i] == ButtonState::BUTTON_REPEAT)
				{
					gameController.triggers[i] = ButtonState::BUTTON_UP;
				}
				else
				{
					gameController.triggers[i] = ButtonState::BUTTON_IDLE;
				}
			}
		}

		// --- GAME CONTROLLER AXIS STATES ---
		int max_positive_threshold = (int)(gameController.max_axis_input_threshold * MAX_AXIS);			//Maybe make it part of the GameController struct.
		int max_negative_threshold = -(int)(gameController.max_axis_input_threshold * MAX_AXIS);
		int min_positive_threshold = (int)(gameController.min_axis_input_threshold * MAX_AXIS);
		int min_negative_threshold = -(int)(gameController.min_axis_input_threshold * MAX_AXIS);

		for (int i = 0; i < NUM_CONTROLLER_AXIS; ++i)
		{
			int axis_value = SDL_GameControllerGetAxis(gameController.id, SDL_GameControllerAxis(i));		//Getting the current axis value of the given joystick.

			if (abs(axis_value) < (int)(gameController.min_axis_input_threshold * MAX_AXIS))
			{
				axis_value = 0;																				// Safety check to compensate for the controller's joystick dead value.
			}

			//LOG("CURRENT AXIS VALUE IS: %d", axis_value);

			if (axis_value > max_positive_threshold)														// The joystick is all the way into the positive (BOTTOM / RIGHT).
			{
				if (gameController.axis[i] == AxisState::AXIS_IDLE)
				{
					gameController.axis[i] = AxisState::POSITIVE_AXIS_DOWN;
				}
				else
				{
					gameController.axis[i] = AxisState::POSITIVE_AXIS_REPEAT;
				}
			}
			else
			{
				if (axis_value < min_positive_threshold && axis_value > min_negative_threshold)			// The joystick is within the min threshold. (No negative axis input)
				{
					if (gameController.axis[i] == AxisState::POSITIVE_AXIS_DOWN || gameController.axis[i] == AxisState::POSITIVE_AXIS_REPEAT)
					{
						gameController.axis[i] = AxisState::POSITIVE_AXIS_RELEASE;
					}
					else
					{
						gameController.axis[i] = AxisState::AXIS_IDLE;
					}
				}
			}

			if (axis_value < max_negative_threshold)														// The joystick is all the way into the negative (TOP / LEFT).
			{
				if (gameController.axis[i] == AxisState::AXIS_IDLE)
				{
					gameController.axis[i] = AxisState::NEGATIVE_AXIS_DOWN;
				}
				else
				{
					gameController.axis[i] = AxisState::NEGATIVE_AXIS_REPEAT;
				}
			}
			else if (axis_value > min_negative_threshold&& axis_value < min_positive_threshold)													// The joystick is not past the positive threshold. (No positive axis input)
			{
				if (gameController.axis[i] == AxisState::NEGATIVE_AXIS_DOWN || gameController.axis[i] == AxisState::NEGATIVE_AXIS_REPEAT)
				{
					gameController.axis[i] = AxisState::NEGATIVE_AXIS_RELEASE;
				}
				else
				{
					gameController.axis[i] = AxisState::AXIS_IDLE;
				}
			}
		}
	}
	else																											// If the game controller has been disconnected.
	{
		if (gameController.id != nullptr)																			// There was a controller that was open, attached and had an ID
		{
			SDL_GameControllerClose(gameController.id);
			gameController.id = nullptr;
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

ButtonState M_Input::GetGameControllerButton(int id) const
{
	if (gameController.id != nullptr)
	{
		return gameController.buttons[id];
	}

	return ButtonState::UNKNOWN_BUTTON;
}

ButtonState M_Input::GetGameControllerTrigger(int id) const
{
	if (gameController.id != nullptr)
	{
		return gameController.triggers[id];
	}

	return ButtonState::UNKNOWN_BUTTON;
}

AxisState M_Input::GetGameControllerAxis(int id) const
{
	if (gameController.id != nullptr)
	{
		return gameController.axis[id];
	}

	return AxisState::UNKNOWN_AXIS;
}

int M_Input::GetGameControllerAxisValue(int id) const
{
	if (gameController.id != nullptr) {
		if(SDL_GameControllerGetAxis(gameController.id, SDL_GameControllerAxis(id)) < -JOYSTICK_THRESHOLD)
		return SDL_GameControllerGetAxis(gameController.id, SDL_GameControllerAxis(id));

		if (SDL_GameControllerGetAxis(gameController.id, SDL_GameControllerAxis(id)) > JOYSTICK_THRESHOLD)
		return SDL_GameControllerGetAxis(gameController.id, SDL_GameControllerAxis(id));
	}
	return 0;
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