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
		static bool pauseScancodeRead = false;

		static RAWINPUT raw;
		UINT size = sizeof(RAWINPUT);

		if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) == (UINT)-1)
		{
			LogError("HandleRawInput(): error happened in GetRawInputData()");
			return;
		}

		//LogInfo("RawInput: dwType: %d, dwSize: %d, hDevice: %d, wParam: %d\n", raw.header.dwType, raw.header.dwSize, raw.header.hDevice, raw.header.wParam);
		if(raw.header.dwType == RIM_TYPEKEYBOARD)
		{
			unsigned int pressed = 0;
			unsigned int scancode = raw.data.keyboard.MakeCode; /* MakeCode is unsigned short. */
			unsigned short flags = raw.data.keyboard.Flags;
			ASSERT(scancode <= 0xff);

			if((flags & RI_KEY_BREAK) == 0)
				pressed = 1;

			if(flags & RI_KEY_E0)
				scancode |= 0xE000;
			else if(flags & RI_KEY_E1)
				scancode |= 0xE100;

			/* The pause scancode is in 2 parts: a WM_INPUT with 0xE11D and one WM_INPUT with 0x45. */
			if(scancode == 0xE11D)
			{
				pauseScancodeRead = true;
			}
			else if(pauseScancodeRead)
			{
				if(scancode == 0x45)
					scancode = (unsigned int)Scancode::sc_pause;
				pauseScancodeRead = false;
			}
			else if(scancode == 0x54)
			{
				/* Alt + print screen return scancode 0x54 but we want it to return 0xE037 because 0x54 will not return a name for the key. */
				scancode = (unsigned int)Scancode::sc_printScreen;
			}
			
			/*
			Some scancodes we can ignore:
			- 0xE11D: first part of the Pause scancode (handled above);
			- 0xE02A: first part of the Print Screen scancode if no Shift, Control or Alt keys are pressed;
			- 0xE02A, 0xE0AA, 0xE036, 0xE0B6: generated in addition of Insert, Delete, Home, End, Page Up, Page Down, Up, Down, Left, Right when num lock is on; or when num lock is off but one or both shift keys are pressed;
			- 0xE02A, 0xE0AA, 0xE036, 0xE0B6: generated in addition of Numpad Divide and one or both Shift keys are pressed;
			- Some of those a break scancode;

			When holding a key down, the pre/postfix (0xE02A) is not repeated.
			*/

			if(scancode == 0xE11D || scancode == 0xE02A || scancode == 0xE0AA || scancode == 0xE0B6 || scancode == 0xE036)
				return;

			// getting a human-readable string
			UINT key = (scancode << 16);
			char buffer[512] = {};
			GetKeyNameText((LONG)key, buffer, 512);treba prerobit

			LogInfo("%s\n", buffer);
		}
	}

	LRESULT OnMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch(msg)
		{
			//case WM_KILLFOCUS: m_engine->getInputSystem().enable(false); break;
			//case WM_SETFOCUS: m_engine->getInputSystem().enable(true); break;
			case WM_CLOSE: PostQuitMessage(0); break;
			//case WM_MOVE:
			//case WM_SIZE: onResize(); break;
			case WM_QUIT: m_finished = true; break;
			case WM_INPUT: HandleRawInput(lparam); break;
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	static LRESULT CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		ASSERT(s_instance != nullptr);
		//if(!s_instance)
			//return DefWindowProc(hwnd, msg, wparam, lparam);

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