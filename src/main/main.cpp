#define VC_EXTRALEAN
#include <windows.h>
#include "core/allocators.h"
#include "core/asserts.h"
#include "core/logs.h"
#include "core/input/input_win.h"
typedef unsigned __int64 QWORD;


namespace Veng
{


class App
{
public:
	App()
	{
		s_instance = this;
	}

	~App()
	{
		s_instance = nullptr;
	}

	void Init()
	{
		CreateMainWindow();

		RegisterRawInput();

		if(!m_windowMode)
			SetFullscreenBorderless();
	}

	void Deinit()
	{

	}

	void Run()
	{
		while(!m_finished)
		{
			Sleep(1000 / 60);
			HandleEvents();
		}
	}

	void HandleEvents()
	{
		MSG msg;
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			OnMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
		}
	}

	int GetExitCode() const { return m_exitCode; }

private:
	void CreateMainWindow()
	{
		HINSTANCE hInst = GetModuleHandle(NULL); //handle to current exe module
		WNDCLASSEX wnd;
		wnd = {};
		wnd.cbSize = sizeof(wnd);
		wnd.style = CS_HREDRAW | CS_VREDRAW; //redraw on horizontal or vertical resize
		wnd.lpfnWndProc = MsgProc; // message process callback function
		wnd.hInstance = hInst; // handle to module
		wnd.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
		wnd.lpszClassName = "App";
		wnd.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		RegisterClassExA(&wnd);

		RECT rect = { 0, 0, 600, 400 };
		AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, FALSE);

		m_hwnd = CreateWindowA("App",
			"App",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			0,
			0,
			rect.right - rect.left,
			rect.bottom - rect.top,
			NULL,
			NULL,
			hInst,
			0);
	}

	void SetFullscreenBorderless()
	{
		HMONITOR hmon = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		if(!GetMonitorInfo(hmon, &mi)) return;

		SetWindowLong(m_hwnd, GWL_STYLE, GetWindowLong(m_hwnd, GWL_STYLE) & ~(WS_CAPTION | WS_THICKFRAME));
		SetWindowLong(m_hwnd,
			GWL_EXSTYLE,
			GetWindowLong(m_hwnd, GWL_EXSTYLE) &
			~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
		SetWindowPos(m_hwnd,
			NULL,
			mi.rcMonitor.left,
			mi.rcMonitor.top,
			mi.rcMonitor.right - mi.rcMonitor.left,
			mi.rcMonitor.bottom - mi.rcMonitor.top,
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}

	void RegisterRawInput()
	{
		const int DEVICE_COUNT = 1;//2;

		RAWINPUTDEVICE Rid[DEVICE_COUNT];

		/*Rid[0].usUsagePage = 0x01;
		Rid[0].usUsage = 0x02;
		Rid[0].dwFlags = RIDEV_NOLEGACY;   // adds HID mouse and also ignores legacy mouse messages
		//if you want to be notified about WM_INPUT_DEVICE_CHANGE add flag RIDEV_DEVNOTIFY
		Rid[0].hwndTarget = 0;*/

		Rid[0].usUsagePage = 0x01;
		Rid[0].usUsage = 0x06;
		Rid[0].dwFlags = RIDEV_NOLEGACY;   // adds HID keyboard and also ignores legacy keyboard messages
		Rid[0].hwndTarget = 0;

		if(RegisterRawInputDevices(Rid, DEVICE_COUNT, sizeof(Rid[0])) == FALSE)
		{
			//DWORD err = GetLastError(); //WHERE CAN I FIND ERROR CODES DESCS ???
			Veng::LogError("RegisterRawInputDevices failed\n");
		}
	}

	void HandleRawInput(LPARAM lParam)
	{
		RAWINPUT raw;
		UINT size = sizeof(RAWINPUT);

		if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) == (UINT)-1)
		{
			LogError("HandleRawInput(): error happened in GetRawInputData()");
			return;
		}

		if(raw.header.dwType == RIM_TYPEKEYBOARD)
		{
			static bool pauseScancodeRead = false;
			bool pressed = false;
			unsigned int scancode = raw.data.keyboard.MakeCode; // MakeCode is USHORT
			unsigned short flags = raw.data.keyboard.Flags;
			ASSERT(scancode <= MAX_MAKECODE);

			if((flags & RI_KEY_BREAK) == 0)
				pressed = true;

			if(flags & RI_KEY_E0)
				scancode |= SC_E0;
			else if(flags & RI_KEY_E1)
				scancode |= SC_E1;

			if(scancode == SC_PAUSE_PART1) //The pause scancode is in 2 parts: a WM_INPUT with 0xE11D and one WM_INPUT with 0x45
			{
				pauseScancodeRead = true;
			}
			else if(pauseScancodeRead)
			{
				if(scancode == SC_PAUSE_PART2)
					scancode = sc_pause;
				pauseScancodeRead = false;
			}
			else if(scancode == sc_alt_printScreen) //Alt + print screen return scancode 0x54 but we want it to return 0xE037 because 0x54 will not return a name for the key
			{
				scancode = sc_printScreen;
			}
			
			//some of those a break scancode, so we ignore them
			if(scancode == SC_PAUSE_PART1 || scancode == SC_IGNORE1 || scancode == SC_IGNORE2 || scancode == SC_IGNORE3 || scancode == SC_IGNORE4)
				return;

			// getting a human-readable string
			//char buffer[512] = {};
			//getScancodeName(scancode, buffer, 512);

			//LogInfo("%s : %s\n", buffer, (pressed) ? "down" : "up");


			//------------------------------------------------------------------------------------
			// GET UTF CHAR
			static unsigned char keyState[256] = {};

			USHORT VKey = raw.data.keyboard.VKey;
			bool e0 = (flags & RI_KEY_E0) != 0;
			bool e1 = (flags & RI_KEY_E1) != 0;

			// these are unasigned but not reserved as of now.
			// this is bad but, you know, we'll fix it if it ever breaks.
			#define VK_LRETURN         0x9E
			#define VK_RRETURN         0x9F

			#define UPDATE_KEY_STATE(key) do { keyState[key] = (flags & 1) ? 0 : 0xff; } while(0)
			// note: we set all bits in the byte if the key is down. 
			// This is becasue windows expects it to be in the high_order_bit (when using it for converting to unicode for example)
			// and I like it to be in the low_order_bit,  
			if(VKey == VK_CONTROL)
			{
				if(e0)	UPDATE_KEY_STATE(VK_RCONTROL);
				else	UPDATE_KEY_STATE(VK_LCONTROL);
				keyState[VK_CONTROL] = keyState[VK_RCONTROL] | keyState[VK_LCONTROL];
			}
			else if(VKey == VK_SHIFT)
			{
				// because why should any api be consistant lol
				// (because we get different scancodes for l/r-shift but not for l/r ctrl etc... but still)
				UPDATE_KEY_STATE(MapVirtualKey(raw.data.keyboard.MakeCode, MAPVK_VSC_TO_VK_EX));
				keyState[VK_SHIFT] = keyState[VK_LSHIFT] | keyState[VK_RSHIFT];
			}
			else if(VKey == VK_MENU)
			{
				if(e0)	UPDATE_KEY_STATE(VK_LMENU);
				else	UPDATE_KEY_STATE(VK_RMENU);
				keyState[VK_MENU] = keyState[VK_RMENU] | keyState[VK_LMENU];
			}
			else if(VKey == VK_RETURN)
			{
				if(e0) UPDATE_KEY_STATE(VK_RRETURN);
				else	UPDATE_KEY_STATE(VK_LRETURN);
				keyState[VK_RETURN] = keyState[VK_RRETURN] | keyState[VK_LRETURN];
			}
			else
			{
				UPDATE_KEY_STATE(VKey);
			}
			#undef UPDATE_KEY_STATE

			if(pressed)
			{
				//char utf8_buffer[32];
				//char *buff = 0;
				//int utf8_len = 0;

				// get unicode.
				wchar_t utf16_buffer[16] = { 0 };
				//simulating altgr, assumes all leftalts is algr
				// which seem to work since ctrl is ignored on US versions of ToUnicode. Bad way of doing it, Not sure how to detect if left alt is altgr though.
				unsigned char ctrl = keyState[VK_CONTROL];
				keyState[VK_CONTROL] |= keyState[VK_RMENU];
				int utf16_len = ToUnicode(VKey, raw.data.keyboard.MakeCode, keyState, utf16_buffer, sizeof(utf16_buffer) / sizeof(utf16_buffer[0]), 0);
				keyState[VK_CONTROL] = ctrl;

				if(utf16_len > 0)
					OutputDebugStringW(utf16_buffer);
			}
		}
	}

	LRESULT OnMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch(msg)
		{
			/*case WM_KILLFOCUS:
				m_engine->getInputSystem().enable(false);
				return 0;
				break;*/
			/*case WM_SETFOCUS:
				m_engine->getInputSystem().enable(true);
				return 0;
				break;*/
			case WM_CLOSE:
				PostQuitMessage(0);
				break;
			//case WM_MOVE:
			//case WM_SIZE: onResize(); break;
			case WM_QUIT:
				m_finished = true;
				break;
			case WM_INPUT:
				HandleRawInput(lparam);
				break;
			case WM_SYSCOMMAND:
				if(wparam == SC_KEYMENU) //Remove beeping sound when ALT + some key is pressed.
					return 0;
				break;
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	static LRESULT CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		ASSERT(s_instance != nullptr);

		return s_instance->OnMessage(hwnd, msg, wparam, lparam);
	}

private:
	HWND m_hwnd;

	int m_exitCode = 0;
	bool m_finished = false;
	bool m_windowMode = true;

	static App* s_instance;
};


App* App::s_instance = nullptr;


}


INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
{
	Veng::App app;
	app.Init();
	app.Run();
	app.Deinit();
	return app.GetExitCode();
}