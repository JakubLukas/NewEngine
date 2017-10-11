#define VC_EXTRALEAN
#include <windows.h>
#include "core/allocators.h"
#include "core/engine.h"
#include "core/asserts.h"
#include "core/logs.h"
#include "core/input/input_win.h"
#include "core/input/input_system.h"


namespace Veng
{


class App
{
public:
	App()
		: m_allocator(m_mainAllocator)
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

		m_engine = Engine::Create(m_allocator);
	}

	void Deinit()
	{
		Engine::Destroy(m_engine, m_allocator);
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
		UINT numDevices;
		GetRawInputDeviceList(
			NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));
		if (numDevices == 0) return;

		RAWINPUTDEVICELIST* deviceList = new RAWINPUTDEVICELIST[numDevices];
		GetRawInputDeviceList(deviceList, &numDevices, sizeof(RAWINPUTDEVICELIST));
		for (UINT i = 0; i < numDevices; ++i)
		{

		}


		const int DEVICE_COUNT = 2;

		RAWINPUTDEVICE Rid[DEVICE_COUNT];

		Rid[0].usUsagePage = 0x01;
		Rid[0].usUsage = 0x02;
		Rid[0].dwFlags = RIDEV_NOLEGACY;   // adds HID mouse and also ignores legacy mouse messages
		//if you want to be notified about WM_INPUT_DEVICE_CHANGE add flag RIDEV_DEVNOTIFY
		Rid[0].hwndTarget = NULL;

		Rid[1].usUsagePage = 0x01;
		Rid[1].usUsage = 0x06;
		Rid[1].dwFlags = RIDEV_NOLEGACY;   // adds HID keyboard and also ignores legacy keyboard messages
		Rid[1].hwndTarget = NULL;

		if(RegisterRawInputDevices(Rid, DEVICE_COUNT, sizeof(Rid[0])) == FALSE)
		{
			//DWORD err = GetLastError(); //WHERE CAN I FIND ERROR CODES DESCS ???
			Veng::LogError("RegisterRawInputDevices failed\n");
		}
	}

	void HandleRawInputKeyboard(inputDeviceHandle deviceHandle, const RAWKEYBOARD& keyboard)
	{
		bool pressed = ((keyboard.Flags & RI_KEY_BREAK) == 0);
		u32 scancodePS2 = Keyboard::getScancodeFromRawInput(&keyboard);

		u8 scUSB = Keyboard::Scancode_USBHID::FromPS2(scancodePS2);
		KeyboardDevice::Button keyCode = (KeyboardDevice::Button)scUSB;
		m_engine->GetInputSystem()->RegisterKeyboardButtonEvent(deviceHandle, keyCode, pressed);

		/*u32 scPS2 = Scancode_PS2::FromUSBHID(scUSB);
		// getting a human-readable string
		char buffer[512] = {};
		getScancodeName(scPS2, buffer, 512);
		//LogInfo("%s : %s\n", buffer, (pressed) ? "down" : "up");

		const i32 BUFFER_SIZE = 4;
		wchar_t utf16_buffer[BUFFER_SIZE] = { 0 };
		i32 readChars = getUTF16TextFromRawInput(&rawKeyboard, utf16_buffer, BUFFER_SIZE);
		ASSERT(readChars < BUFFER_SIZE - 1);
		if(readChars > 0)
		OutputDebugStringW(utf16_buffer);*/
	}

	void HandleRawInputMouse(inputDeviceHandle deviceHandle, const RAWMOUSE& mouse)
	{
		if (mouse.usFlags & MOUSE_ATTRIBUTES_CHANGED)
		{
			//Mouse attributes changed; application needs to query the mouse attributes.
			ASSERT2(false, "what does this mean ?");
		}

		if (mouse.usFlags & MOUSE_MOVE_RELATIVE)
		{
			Vector3 axisMov{
				(float)mouse.lLastX,
				(float)mouse.lLastY,
				0.0f
			};
			m_engine->GetInputSystem()->RegisterMouseAxisEvent(deviceHandle, MouseDevice::Axis::Movement, axisMov);
		}
		else if (mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
		{
			ASSERT2(false, "Mouse movement should be relative");
		}
		else

			if (mouse.usFlags & MOUSE_VIRTUAL_DESKTOP)
			{
				//Mouse coordinates are mapped to the virtual desktop (for a multiple monitor system).
				ASSERT2(false, "Mouse movement is mapped to virtual desktop");
			}

		if (mouse.usButtonFlags & RI_MOUSE_WHEEL)
		{
			Vector3 axisWheel{
				(float)mouse.usButtonData,
				0.0f,
				0.0f
			};
			m_engine->GetInputSystem()->RegisterMouseAxisEvent(deviceHandle, MouseDevice::Axis::Wheel, axisWheel);
		}
		else
		{
			bool pressed;
			Veng::MouseDevice::Button code;
			if (Mouse::getButtonCodeFromRawInput(&mouse, pressed, code))
			{
				m_engine->GetInputSystem()->RegisterMouseButtonEvent(deviceHandle, code, pressed);
			}
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

		inputDeviceHandle deviceHandle = (inputDeviceHandle)(raw.header.hDevice);

		switch (raw.header.dwType)
		{
		case RIM_TYPEMOUSE:
			HandleRawInputMouse(deviceHandle, raw.data.mouse);
			break;
		case RIM_TYPEKEYBOARD:
			HandleRawInputKeyboard(deviceHandle, raw.data.keyboard);
			break;
		case RIM_TYPEHID:
			//handle HID
			break;
		default:
			ASSERT2(false, "Unknown type of raw input device");
			break;
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
			/*case WM_MOVE:
			case WM_SIZE:
				onResize();
				break;*/
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

	MainAllocator m_mainAllocator;
	HeapAllocator m_allocator;
	Engine* m_engine;

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