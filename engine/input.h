
#pragma once

#include "master.h"


//======================================================================

struct timer_ {

	enum {
		MAX_STEPS = 5,
	};

	__int32 frame_count;
	__int64 counter;
	__int64 prev_counter;
	__int64 frequency;
	float delta_time;
	float real_delta_time;
	double fixed_step;
	double accumulator;
};

struct mouse_ {

	__int32 delta_x;
	__int32 delta_y;

	__int32 previous_delta_x;
	__int32 previous_delta_y;

};

struct user_input_ {

	enum {

		FORWARD,
		BACK,
		LEFT,
		RIGHT,
		JUMP,
		FIRE,
		USE,
		PLAYER_ONE,
		PLAYER_TWO,
		PLAYER_THREE,
		PLAYER_FOUR,
		WEAPON_NEXT,
		WEAPON_PREV,
	};

	unsigned __int32 input_mask;
	mouse_ mouse;
};

//======================================================================

void Initialise_Input(timer_&, user_input_&);
void Update_Mouse_Position(bool, mouse_&);
__int32 Update_Timer(timer_&);
void Cursor_Control(HWND, HCURSOR);

