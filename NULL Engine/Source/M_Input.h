#ifndef __M_INPUT_H__
#define __M_INPUT_H__

#include "Module.h"

class ParsonNode;
typedef unsigned int uint;

enum class KEY_STATE
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
	
	M_Input(bool is_active = true);
	~M_Input();

	bool			Init					(ParsonNode& config) override;
	UPDATE_STATUS	PreUpdate				(float dt) override;
	UPDATE_STATUS	Update					(float dt) override;
	UPDATE_STATUS	PostUpdate				(float dt) override;
	bool			CleanUp					() override;

	bool			LoadConfiguration		(ParsonNode& root) override;
	bool			SaveConfiguration		(ParsonNode& root) const override;

public:
	KEY_STATE		GetKey					(int id) const;
	KEY_STATE		GetMouseButton			(int id) const;
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

private:
	KEY_STATE*		keyboard;
	KEY_STATE		mouse_buttons[MAX_MOUSE_BUTTONS];
	uint			max_num_scancodes;

	int				mouse_x;
	int				mouse_y;
	int				mouse_z;
	int				mouse_x_motion;
	int				mouse_y_motion;
	int				mouse_x_wheel;
	int				mouse_y_wheel;

	int				prev_x_mouse_pos;
	int				prev_y_mouse_pos;
};

#endif // !__M_INPUT_H__