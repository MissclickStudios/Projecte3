#ifndef __M_INPUT_H__
#define __M_INPUT_H__

#include "Module.h"
#include <vector>

class ParsonNode;
typedef unsigned int uint;

enum class KeyState
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
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
	
	void			AddModuleToProcessInput(Module* module);												//Add a module that needs SDL_Events inputs info

private:
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

	std::vector<Module*> ModulesProcessInput;
};

#endif // !__M_INPUT_H__