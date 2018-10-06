#include "windows.h"
#include "input.h"
#include "render_front.h"


#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif
#ifndef HID_USAGE_GENERIC_KEYBOARD
#define HID_USAGE_GENERIC_KEYBOARD     ((USHORT) 0x06)
#endif


//======================================================================

struct window_ {

	bool is_running;
	bool has_focus;
	unsigned __int32 input_mask;
	__int32 delta_mouse_x;
	__int32 delta_mouse_y;
	HWND handle;
	HCURSOR handle_cursor;
};

window_ Window;

//======================================================================

/*
==================
==================
*/
LRESULT CALLBACK Window_Callback(

	_In_ HWND   window_handle,
	_In_ UINT   message,
	_In_ WPARAM w_paramater,
	_In_ LPARAM l_paramater
)
{
	switch (message) {

		case WM_ACTIVATEAPP: {
			//OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;

		case WM_PAINT: {
			ValidateRect(window_handle, NULL);
			return 0;
		} break;

		case WM_DESTROY: {
			Window.is_running = false;
			//OutputDebugStringA("WM_DESTROY\n");
		} break;

		case WM_CLOSE: {
			Window.is_running = false;
			//OutputDebugStringA("WM_CLOSE\n");
		} break;

		//case WM_SETCURSOR: {
		//	if (LOWORD(l_paramater) == HTCLIENT) {

		//	}
		//} break;

		case WM_INPUT:
		{
			const __int32 size_buffer = 64;
			UINT dwSize = size_buffer;
			BYTE lpb[size_buffer];

			UINT result = GetRawInputData(

				(HRAWINPUT)l_paramater,
				RID_INPUT,
				lpb,
				&dwSize,
				(__int32)sizeof(RAWINPUTHEADER)
			);

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE)
			{
				Window.delta_mouse_x = raw->data.mouse.lLastX;
				Window.delta_mouse_y = raw->data.mouse.lLastY;
				__int16 scroll_value = (__int16)raw->data.mouse.usButtonData;
				//printf_s("MOUSE %i \n", (__int16)raw->data.mouse.usButtonData);

				//printf_s("MOUSE WHEEL: %i \n", mouse_wheel);
				unsigned __int32 down_bit = (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) != 0x0;
				unsigned __int32 up_bit = (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) != 0x0;
				unsigned __int32 mouse_wheel_bit = (raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL) != 0x0;



				Window.input_mask |= down_bit << user_input_::FIRE;
				Window.input_mask ^= up_bit << user_input_::FIRE;
				const __int32 shift = (scroll_value < 0) ? user_input_::WEAPON_PREV : user_input_::WEAPON_NEXT;
				//Window.input_mask |= mouse_wheel_bit << shift;

				//printf_s("MOUSE WHEEL: %i \n", mouse_wheel_bit << shift);
			}
			if (raw->header.dwType == RIM_TYPEKEYBOARD)
			{
				UINT event = raw->data.keyboard.Message;

				unsigned __int32 key_mask = 0x0;

				key_mask |= (raw->data.keyboard.VKey == 'W') << user_input_::FORWARD;
				key_mask |= (raw->data.keyboard.VKey == 'S') << user_input_::BACK;
				key_mask |= (raw->data.keyboard.VKey == 'A') << user_input_::LEFT;
				key_mask |= (raw->data.keyboard.VKey == 'D') << user_input_::RIGHT;
				key_mask |= (raw->data.keyboard.VKey == ' ') << user_input_::JUMP;
				key_mask |= (raw->data.keyboard.VKey == 'Q') << user_input_::USE;
				key_mask |= (raw->data.keyboard.VKey == '1') << user_input_::PLAYER_ONE;
				key_mask |= (raw->data.keyboard.VKey == '2') << user_input_::PLAYER_TWO;
				key_mask |= (raw->data.keyboard.VKey == '3') << user_input_::PLAYER_THREE;
				key_mask |= (raw->data.keyboard.VKey == '4') << user_input_::PLAYER_FOUR;

				Window.input_mask |= (event == WM_KEYDOWN) ? key_mask : 0x0;
				Window.input_mask ^= (event == WM_KEYUP) ? key_mask : 0x0;

				Window.is_running = (raw->data.keyboard.VKey != VK_ESCAPE);
			}

			break;
		}

		case WM_SETFOCUS: {

			Window.has_focus = true;
			Cursor_Control(window_handle, Window.handle_cursor);

		} break;

		case WM_KILLFOCUS: {

			Window.has_focus = false;
			ClipCursor(NULL);
			while (ShowCursor(TRUE) < 0) {};

		} break;

		case WM_CREATE: {

			Cursor_Control(window_handle, Window.handle_cursor);

		} break;

	}

	return DefWindowProc(window_handle, message, w_paramater, l_paramater);;
}

/*
==================
==================
*/
void Initialise_Window(FILE* fp, display_& display){

	{
		HINSTANCE instance_handle = GetModuleHandle(0);

		WNDCLASS window_class = {};
		window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		window_class.hInstance = instance_handle;
		window_class.lpszClassName = L"lamorna engine class";
		window_class.lpfnWndProc = Window_Callback;

		BYTE cursor_mask_AND[] = { 0xFF };
		BYTE cursor_mask_XOR[] = { 0x00 };
		window_class.hCursor = CreateCursor(NULL, 0, 0, 1, 1, cursor_mask_AND, cursor_mask_XOR);

		ATOM return_value = RegisterClass(&window_class);

		if (return_value == 0) {
			printf_s("FAILED to register windows class\n");
			exit(0);
		}

		Window.is_running = false;
		Window.handle_cursor = window_class.hCursor;

		DWORD window_style = (WS_OVERLAPPEDWINDOW | WS_VISIBLE) ^ WS_THICKFRAME;

		RECT display_rect;
		display_rect.left = 0;
		display_rect.right = display_::WINDOW_WIDTH;
		display_rect.top = 0;
		display_rect.bottom = display_::WINDOW_HEIGHT;

		AdjustWindowRect(&display_rect, window_style, FALSE);

		Window.handle = CreateWindowEx(
			0,
			window_class.lpszClassName,
			L"Lamorna Engine",
			window_style,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			display_rect.right - display_rect.left,
			display_rect.bottom - display_rect.top,
			0,
			0,
			instance_handle,
			0
		);

		if (Window.handle == NULL) {
			printf_s("FAILED to create window\n");
			exit(0);
		}
	}
	// ----------------------------------------------------------------------------------------------------------
	{	// initialise D3D9 interface
		IDirect3D9* d3d9_interface = Direct3DCreate9(D3D_SDK_VERSION);

		if (d3d9_interface == NULL) {
			printf_s("FAILED to create D3D interface\n");
			exit(0);
		}

		D3DPRESENT_PARAMETERS d3d_parameters;
		ZeroMemory(&d3d_parameters, sizeof(d3d_parameters));

		d3d_parameters.BackBufferCount = 1;
		d3d_parameters.BackBufferWidth = display_::WINDOW_WIDTH;
		d3d_parameters.BackBufferHeight = display_::WINDOW_HEIGHT;
		d3d_parameters.BackBufferFormat = D3DFMT_A8R8G8B8;		// OR let windows set just in case?
		d3d_parameters.Windowed = TRUE;
		d3d_parameters.hDeviceWindow = Window.handle;
		d3d_parameters.SwapEffect = D3DSWAPEFFECT_COPY;
		d3d_parameters.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

		display.d3d9_device = NULL;
		HRESULT h_result = d3d9_interface->CreateDevice(
			D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			NULL,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&d3d_parameters,
			&display.d3d9_device
		);

		if (h_result != D3D_OK) {
			fprintf(fp, "d3d device HANDLE NOT RETURNED\n");
			exit(0);
		}

		display.handle_window = Window.handle;
	}
	// ----------------------------------------------------------------------------------------------------------
	{

		Window.is_running = true;
		Window.has_focus = true;
		Window.input_mask = 0x0;
		Window.delta_mouse_x = 0;
		Window.delta_mouse_y = 0;
	}
	// ----------------------------------------------------------------------------------------------------------
	{
		const __int32 n_raw_input_devices = 2;
		RAWINPUTDEVICE Rid[n_raw_input_devices];

		Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		Rid[0].dwFlags = RIDEV_NOLEGACY; // | RIDEV_INPUTSINK;
		Rid[0].hwndTarget = Window.handle;

		Rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		Rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		Rid[1].dwFlags = RIDEV_NOLEGACY; // | RIDEV_INPUTSINK;
		Rid[1].hwndTarget = Window.handle;

		bool result = RegisterRawInputDevices(Rid, n_raw_input_devices, (unsigned __int32)sizeof(Rid[0]));

		if (!result) {

			printf_s("raw input devices FAILED to register \n");
			exit(0);
		}

	}
}

/*
==================
==================
*/
bool Windows_Messages(user_input_& user_input) {

	Window.delta_mouse_x = 0;
	Window.delta_mouse_y = 0;

	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {

		if (message.message == WM_QUIT) {
			Window.is_running = false;
		}
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	if (Window.has_focus) {

		user_input.input_mask = Window.input_mask;
		user_input.mouse.previous_delta_x = user_input.mouse.delta_x;
		user_input.mouse.previous_delta_y = user_input.mouse.delta_y;
		user_input.mouse.delta_x = Window.delta_mouse_x;
		user_input.mouse.delta_y = Window.delta_mouse_y;
	}

	unsigned __int32 bit_clear_mouse_wheel = ~((0x1 << user_input_::WEAPON_NEXT) | (0x1 << user_input_::WEAPON_PREV));
	Window.input_mask &= bit_clear_mouse_wheel;

	return Window.is_running;
}

/*
==================
==================
*/
void Get_Cursor_Position(point& point) {

	POINT temp;	
	GetCursorPos(&temp);
	point.x = temp.x;
	point.y = temp.y;
}
/*
==================
==================
*/
void Set_Cursor_Position(const __int32 x, const __int32 y) {

	SetCursorPos(x, y);
}
/*
==================
==================
*/
void Client_To_Screen(point& point) {

	POINT temp;
	temp.x = point.x;
	temp.y = point.y;
	ClientToScreen(Window.handle, &temp);
	point.x = temp.x;
	point.y = temp.y;
}

/*
==================
==================
*/
void DIRECT_SOUND_Set_Cooperative_Level(LPDIRECTSOUND8 DS_interface) {

	DS_interface->SetCooperativeLevel(Window.handle, DSSCL_NORMAL);
}

/*
==================
==================
*/
void Query_Performance_Counter(__int64& in) {

	QueryPerformanceCounter((LARGE_INTEGER*)&in);
}
/*
==================
==================
*/
void Query_Performance_Frequency(__int64& in) {

	QueryPerformanceFrequency((LARGE_INTEGER*)&in);
}

/*
==================
==================
*/
void Windows_Shutdown(display_& display) {


	display.d3d9_device->Release();

}