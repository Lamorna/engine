

#pragma once

//======================================================================

struct timer;
struct display_;
struct user_input;
struct thread_pool;
struct _iobuf;
typedef _iobuf FILE;

//======================================================================

struct point {

	__int32 x;
	__int32 y;
};

struct rect {

	__int32 left;
	__int32 top;
	__int32 right;
	__int32 bottom;
};

//======================================================================

struct user_input_;

void Get_Cursor_Position(point&);
void Set_Cursor_Position(const __int32 x, const __int32 y);
void Client_To_Screen(point&);

void Initialise_Window(FILE*, display_&);
void Query_Performance_Frequency(__int64&);
void Query_Performance_Counter(__int64&);
bool Windows_Messages(user_input_&);
void Windows_Shutdown(display_&);

struct IDirectSound8;
typedef IDirectSound8 *LPDIRECTSOUND8;
void DIRECT_SOUND_Set_Cooperative_Level(LPDIRECTSOUND8);
