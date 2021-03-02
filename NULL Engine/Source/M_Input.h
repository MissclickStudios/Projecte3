#ifndef __M_INPUT_H__
#define __M_INPUT_H__

#include "Module.h"

#define NUM_CONTROLLER_BUTTONS 15
#define NUM_CONTROLLER_AXIS 4
#define NUM_CONTROLLER_TRIGGERS 2

#define MAX_AXIS 32767								
#define CONTROLLER_INDEX 0							
#define TRIGGER_INDEX 4								

#define LEFT_TRIGGER 0								
#define RIGHT_TRIGGER 1								

struct _SDL_GameController;
class ParsonNode;
typedef unsigned int uint;

enum class KeyState
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};

enum class ButtonState								
{
	BUTTON_IDLE,
	BUTTON_DOWN,
	BUTTON_REPEAT,
	BUTTON_UP,
	UNKNOWN_BUTTON									
};

enum class AxisState
{								
	AXIS_IDLE,
	POSITIVE_AXIS_DOWN,
	POSITIVE_AXIS_REPEAT,
	POSITIVE_AXIS_RELEASE,
	NEGATIVE_AXIS_DOWN,
	NEGATIVE_AXIS_REPEAT,
	NEGATIVE_AXIS_RELEASE,
	UNKNOWN_AXIS								
};


struct GameController
{
	_SDL_GameController*    id;							
	int						index;						

	ButtonState*			buttons;
	ButtonState*			triggers;					
	AxisState*				axis;

	float					max_axis_input_threshold;	
	float					min_axis_input_threshold;	
};

#define MAX_MOUSE_BUTTONS 5

class M_Input : public Module
{
public:
	
	M_Input(bool isActive = true);
	~M_Input();

	bool			Init					(ParsonNode& config) override;
	UpdateStatus	PreUpdate				(float dt) override;
	UpdateStatus	Update					(float dt) override;
	UpdateStatus	PostUpdate				(float dt) override;
	bool			CleanUp					() override;

	bool			LoadConfiguration		(ParsonNode& root) override;
	bool			SaveConfiguration		(ParsonNode& root) const override;

public:
	KeyState		GetKey					(int id) const;
	KeyState		GetMouseButton			(int id) const;
	uint			GetMaxNumScancodes		() const;

	int				GetMouseX				() const;
	int				GetMouseY				() const;
	int				GetMouseZ				() const;
	int				GetMouseXMotion			() const;
	int				GetMouseYMotion			() const;
	int				GetMouseXMotionFromSDL	() const;
	int				GetMouseYMotionFromSDL	() const;
	int				GetMouseXWheel			() const;
	int				GetMouseYWheel			() const;

	bool			WindowSizeWasManipulated(Uint8 windowEvent) const;										// Uint8 is an SDL typedef for unsigned char.

	void			CheckGameControllerState();

private:

	GameController  gameController;
	
	KeyState*		keyboard;
	KeyState		mouseButtons[MAX_MOUSE_BUTTONS];
	uint			maxNumScancodes;

	int				mouseX;
	int				mouseY;
	int				mouseZ;
	int				mouseMotionX;
	int				mouseMotionY;
	int				mouseWheelX;
	int				mouseWheelY;

	int				prevMousePosX;
	int				prevMousePosY;
};

#endif // !__M_INPUT_H__