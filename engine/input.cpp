#include "input.h"
#include "windows.h"

/*
==================
==================
*/
void Cursor_Control(HWND window_handle, HCURSOR handle_cursor) {

	RECT rect;
	GetWindowRect(window_handle, &rect);

	__int32 centre_x = (rect.left + rect.right) / 2;
	__int32 centre_y = (rect.top + rect.bottom) / 2;

	__int32 border_x = (rect.right - rect.left) / 4;
	__int32 border_y = (rect.bottom - rect.top) / 4;
	rect.left += border_x;
	rect.right -= border_x;
	rect.top += border_y;
	rect.bottom -= border_y;

	SetCursor(handle_cursor);
	SetCursorPos(centre_x, centre_y);
	ClipCursor(&rect);

	while (ShowCursor(FALSE) > -1) {};
}


/*
==================
==================
*/
void Initialise_Input(timer_& timer, user_input_& user_input) {

	// ----------------------------------------------------------------------------------------------------------
	{	// initialise timer
		timer.frame_count = 0;
		timer.delta_time = 0.0f;
		timer.fixed_step = 1.0 / 60.0;
		timer.accumulator = 0.0;
		Query_Performance_Frequency(timer.frequency);
		Query_Performance_Counter(timer.prev_counter);
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		user_input.input_mask = 0x0;
		user_input.mouse.delta_x = 0;
		user_input.mouse.delta_y = 0;
	}
}

/*
==================
==================
*/
__int32 Update_Timer(timer_& timer) {

	Query_Performance_Counter(timer.counter);
	double delta = (timer.counter - timer.prev_counter) / (double)timer.frequency;
	timer.real_delta_time = (float)delta;
	timer.prev_counter = timer.counter;
	timer.accumulator += delta;
	timer.delta_time = (float)timer.fixed_step;
	__int32 n_steps = (__int32)(timer.accumulator / timer.fixed_step);
	n_steps = min(n_steps, timer_::MAX_STEPS);
	return n_steps;
}

