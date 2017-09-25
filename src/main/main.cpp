#include "core/allocators.h"
#include "core/asserts.h"
#include "core/logs.h"
#include <windows.h>


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

		//if(!m_window_mode) setFullscreenBorderless();

		RAWINPUTDEVICE Rid;
		Rid.usUsagePage = 0x01;
		Rid.usUsage = 0x02;
		Rid.dwFlags = 0;
		Rid.hwndTarget = 0;
		RegisterRawInputDevices(&Rid, 1, sizeof(Rid));

		if(m_windowMode)
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
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			OnMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
		}
	}

	int GetExitCode() const { return m_exitCode; }

private:
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
		RAWINPUTDEVICE Rid[2];

		Rid[0].usUsagePage = 0x01;
		Rid[0].usUsage = 0x02;
		Rid[0].dwFlags = RIDEV_NOLEGACY;   // adds HID mouse and also ignores legacy mouse messages
		//if you want to be notified about WM_INPUT_DEVICE_CHANGE add flag RIDEV_DEVNOTIFY
		Rid[0].hwndTarget = 0;

		Rid[1].usUsagePage = 0x01;
		Rid[1].usUsage = 0x06;
		Rid[1].dwFlags = RIDEV_NOLEGACY;   // adds HID keyboard and also ignores legacy keyboard messages
		Rid[1].hwndTarget = 0;

		if(RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
		{
			//DWORD err = GetLastError(); //WHERE I CAN FIND ERROR CODES DESC
		}
	}

	void HandleRawInput(LPARAM lparam)
	{
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
	Veng::LogInfo("test %d\n", 1);
	Veng::App app;
	app.Init();
	app.Run();
	app.Deinit();
	return app.GetExitCode();
}