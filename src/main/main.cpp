#include "app.h"

#define VC_EXTRALEAN
#include <windows.h>
#include "core/allocators.h"
#include "core/engine.h"
#include "core/asserts.h"
#include "core/logs.h"
#include "core/input/input_win.h"
#include "core/input/input_system.h"
#include "core/array.h"
#include "core/string.h"
#include <hidsdi.h>


#include "editor.h"


namespace Veng
{


class AppImpl : public App
{
public:
	AppImpl()
		: m_allocator(m_mainAllocator)
	{
		s_instance = this;
	}

	~AppImpl() override
	{
		s_instance = nullptr;
	}


	windowHandle CreateSubWindow() override
	{
		return CreateSubWindow({ 200, 200 });
	}

	windowHandle CreateSubWindow(WindowSize size) override
	{
		if (m_subHwndSize == MAX_SUB_WINDOWS)
			return INVALID_WINDOW_HANDLE;

		HINSTANCE hInst = GetModuleHandle(NULL); //handle to current exe module

		HWND hwnd = CreateWindow(
			WINDOW_CLASS_NAME,
			"",
			WS_CHILD | WS_OVERLAPPED | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE,
			20,
			20,
			size.width,
			size.height,
			m_hwnd,
			NULL,
			hInst,
			NULL);

		if (hwnd != NULL)
		{
			m_subHwnd[m_subHwndSize++] = hwnd;
			return (windowHandle)hwnd;
		}
		else
		{
			DWORD err = GetLastError();
			return INVALID_WINDOW_HANDLE;
		}
	}

	bool DestroySubWindow(windowHandle hwnd) override
	{
		BOOL result = DestroyWindow((HWND)hwnd);
		if(result != 0)
		{
			int i = 0;
			for(; i < m_subHwndSize; ++i)
			{
				if(m_subHwnd[i] == hwnd)
					break;
			}
			for(; i < m_subHwndSize - 1; ++i)
			{
				m_subHwnd[i] = m_subHwnd[i + 1];
			}

			m_subHwnd[--m_subHwndSize] = NULL;
			return true;
		}
		else
		{
			DWORD err = GetLastError();
			return false;
		}
	}

	void DockSubWindow(windowHandle hwnd) override
	{
		SetWindowLong((HWND)hwnd, WS_CHILD | GWL_STYLE, WS_OVERLAPPED | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE);
		SetParent((HWND)hwnd, m_hwnd);
	}

	void UndockSubWindow(windowHandle hwnd) override
	{
		SetWindowLong((HWND)hwnd, GWL_STYLE, WS_OVERLAPPED | WS_THICKFRAME | WS_CAPTION | WS_VISIBLE);
		SetParent((HWND)hwnd, NULL);
	}


	windowHandle GetMainWindowHandle() const override
	{
		return m_hwnd;
	}

	WindowSize GetWindowSize(windowHandle hwnd) const override
	{
		RECT rect;
		ASSERT(GetClientRect((HWND)hwnd, &rect));
		return{
			(u32)(rect.right - rect.left),
			(u32)(rect.bottom - rect.top)
		};
	}


	void Init()
	{
		m_editor = Editor::Create(m_allocator, *this);

		CreateMainWindow();

		if(!m_windowMode)
			SetFullscreenBorderless();

		m_editor->Init();
		WindowSize size = GetWindowSize(m_hwnd);////////////////////////////
		m_editor->Resize(m_hwnd, size.width, size.height);

		RegisterRawInput();

	}

	void Deinit()
	{
		m_editor->Deinit();
		Editor::Destroy(m_editor, m_allocator);
	}

	void Run()
	{
		while(!m_finished)
		{
			static const float frameRate = 1000.0f / 60.0f;
			Sleep((DWORD)frameRate);//TODO ////////////////////////////////////////////

			m_editor->Update(frameRate);
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
	void CreateMainWindow()
	{
		HINSTANCE hInst = GetModuleHandle(NULL); //handle to current exe module

		WNDCLASSEX wnd = {};
		wnd.cbSize = sizeof(wnd);
		wnd.style = CS_HREDRAW | CS_VREDRAW; //redraw on horizontal or vertical resize
		wnd.lpfnWndProc = MsgProc; // message process callback function
		wnd.hInstance = hInst; // handle to module
		wnd.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
		wnd.lpszClassName = WINDOW_CLASS_NAME;
		wnd.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		ASSERT(RegisterClassEx(&wnd));

		RECT rect = { 0, 0, 600, 400 };
		ASSERT(AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VISIBLE, FALSE));

		m_hwnd = CreateWindow(
			WINDOW_CLASS_NAME,
			WINDOW_CLASS_NAME,
			WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			rect.right - rect.left,
			rect.bottom - rect.top,
			NULL,
			NULL,
			hInst,
			NULL);
		ASSERT(m_hwnd != 0);
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
		//REGISTER TO ENGINE
		UINT numDevices;
		if (GetRawInputDeviceList(NULL, &numDevices, sizeof(RAWINPUTDEVICELIST)) != 0)
		{
			Log(LogType::Error, "RegisterRawInputDevices failed\n");
			return;
		}
		if (numDevices == 0) return;

		Array<RAWINPUTDEVICELIST> deviceList(m_allocator);
		deviceList.Resize(numDevices);
		if (GetRawInputDeviceList(&deviceList[0], &numDevices, sizeof(RAWINPUTDEVICELIST)) == -1)
		{
			Log(LogType::Error, "RegisterRawInputDevices failed\n");
			return;
		}

		for (UINT i = 0; i < numDevices; ++i)
		{
			const RAWINPUTDEVICELIST& device = deviceList[i];
			
			String deviceName(m_allocator);
			GetNameOfDevice(device.hDevice, deviceName);

			inputDeviceHandle deviceHandle = (inputDeviceHandle)(device.hDevice);
			switch (device.dwType)
			{
			case RIM_TYPEMOUSE:
				m_editor->RegisterDevice(deviceHandle, InputDeviceCategory::Mouse, deviceName);
				POINT mousePos;
				GetCursorPos(&mousePos);////////////////////////////////////////////////////////////////////////
				m_editor->RegisterAxisEvent(deviceHandle, MouseDevice::Axis::Movement, { (float)mousePos.x, (float)mousePos.y, 0.0f });
				break;
			case RIM_TYPEKEYBOARD:
				m_editor->RegisterDevice(deviceHandle, InputDeviceCategory::Keyboard, deviceName);
				break;
			case RIM_TYPEHID:
			{
				USHORT usage = GetHIDType(device.hDevice);
				switch (usage)
				{
				case HID_USAGE_GENERIC_GAMEPAD:
					m_editor->RegisterDevice(deviceHandle, InputDeviceCategory::Gamepad, deviceName);
					break;
				}
			}
			break;
			default:
				Log(LogType::Info, "Unregistered HID device. handle: 0x%08X, type: 0x%04X\n", device.hDevice, device.dwType);
				break;
			}
		}


		//REGISTER TO WINAPI
		const int DEVICE_COUNT = 3;
		RAWINPUTDEVICE rid[DEVICE_COUNT];

		//mouse
		rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		rid[0].dwFlags = RIDEV_DEVNOTIFY; //if RIDEV_NOLEGACY is used, window will be unresponsive
		rid[0].hwndTarget = m_hwnd;

		//keyboard
		rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		rid[1].dwFlags = RIDEV_DEVNOTIFY | RIDEV_NOLEGACY;
		rid[1].hwndTarget = m_hwnd;

		//gamepad
		rid[2].usUsagePage = HID_USAGE_PAGE_GENERIC;
		rid[2].usUsage = HID_USAGE_GENERIC_GAMEPAD;
		rid[2].dwFlags = RIDEV_DEVNOTIFY;
		rid[2].hwndTarget = m_hwnd;

		if(RegisterRawInputDevices(rid, DEVICE_COUNT, sizeof(RAWINPUTDEVICE)) == FALSE)
		{
			Log(LogType::Error, "RegisterRawInputDevices failed\n");
		}
	}

	void HandleRawInputKeyboard(HANDLE hDevice, const RAWKEYBOARD& keyboard)
	{
		inputDeviceHandle deviceHandle = (inputDeviceHandle)(hDevice);

		bool pressed = ((keyboard.Flags & RI_KEY_BREAK) == 0);
		u32 scancodePS2 = Keyboard::getScancodeFromRawInput(&keyboard);

		u8 scUSB = Keyboard::Scancode_USBHID::FromPS2(scancodePS2);
		KeyboardDevice::Button keyCode = (KeyboardDevice::Button)scUSB;
		m_editor->RegisterButtonEvent(deviceHandle, keyCode, pressed);

		u32 scPS2 = Keyboard::Scancode_PS2::FromUSBHID(scUSB);
		char buffer[512] = {};
		Keyboard::getScancodeName(scPS2, buffer, 512); // getting a human-readable string
		Log(LogType::Info, "%s : %s\n", buffer, (pressed) ? "down" : "up");

		/*const i32 BUFFER_SIZE = 4;
		wchar_t utf16_buffer[BUFFER_SIZE] = { 0 };
		i32 readChars = getUTF16TextFromRawInput(&rawKeyboard, utf16_buffer, BUFFER_SIZE);
		ASSERT(readChars < BUFFER_SIZE - 1);
		if(readChars > 0)
		OutputDebugStringW(utf16_buffer);*/
	}

	void HandleRawInputMouse(HANDLE hDevice, const RAWMOUSE& mouse)
	{
		inputDeviceHandle deviceHandle = (inputDeviceHandle)(hDevice);

		if (mouse.usFlags & MOUSE_ATTRIBUTES_CHANGED)
		{
			//Mouse attributes changed; application needs to query the mouse attributes.
			ASSERT2(false, "what does this mean ?");
		}

		if (mouse.usFlags & MOUSE_VIRTUAL_DESKTOP)
		{
			//Mouse coordinates are mapped to the virtual desktop (for a multiple monitor system).
			ASSERT2(false, "Mouse movement is mapped to virtual desktop");
		}

		if (mouse.lLastX != 0 && mouse.lLastY != 0)
		{
			Vector3 axisMov{
				(float)mouse.lLastX,
				(float)mouse.lLastY,
				0.0f
			};
			m_editor->RegisterAxisEvent(deviceHandle, MouseDevice::Axis::Movement, axisMov);
			return;
		}
		else if (mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
		{
			ASSERT2(false, "Mouse movement should be relative");
			return;
		}

		if (mouse.usButtonFlags & RI_MOUSE_WHEEL)
		{
			Vector3 axisWheel{
				(float)mouse.usButtonData,
				0.0f,
				0.0f
			};
			m_editor->RegisterAxisEvent(deviceHandle, MouseDevice::Axis::Wheel, axisWheel);
			return;
		}
		else
		{
			if (mouse.usButtonFlags != Mouse::BUTTON_NONE)
			{
				if((mouse.usButtonFlags & RI_MOUSE_BUTTON_1_DOWN) != 0)
					m_editor->RegisterButtonEvent(deviceHandle, MouseDevice::Button::Left, true);
				if((mouse.usButtonFlags & RI_MOUSE_BUTTON_1_UP) != 0)
					m_editor->RegisterButtonEvent(deviceHandle, MouseDevice::Button::Left, false);

				if ((mouse.usButtonFlags & RI_MOUSE_BUTTON_2_DOWN) != 0)
					m_editor->RegisterButtonEvent(deviceHandle, MouseDevice::Button::Right, true);
				if ((mouse.usButtonFlags & RI_MOUSE_BUTTON_2_UP) != 0)
					m_editor->RegisterButtonEvent(deviceHandle, MouseDevice::Button::Right, false);

				if ((mouse.usButtonFlags & RI_MOUSE_BUTTON_3_DOWN) != 0)
					m_editor->RegisterButtonEvent(deviceHandle, MouseDevice::Button::Middle, true);
				if ((mouse.usButtonFlags & RI_MOUSE_BUTTON_3_UP) != 0)
					m_editor->RegisterButtonEvent(deviceHandle, MouseDevice::Button::Middle, false);

				if ((mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN) != 0)
					m_editor->RegisterButtonEvent(deviceHandle, MouseDevice::Button::Extra4, true);
				if ((mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP) != 0)
					m_editor->RegisterButtonEvent(deviceHandle, MouseDevice::Button::Extra4, false);

				if ((mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN) != 0)
					m_editor->RegisterButtonEvent(deviceHandle, MouseDevice::Button::Extra5, true);
				if ((mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP) != 0)
					m_editor->RegisterButtonEvent(deviceHandle, MouseDevice::Button::Extra5, false);
			}
		}
	}

	void HandleRawInputHID(HANDLE hDevice, const RAWHID& hid)
	{
		USHORT usage = GetHIDType(hDevice);
		switch (usage)
		{
		case HID_USAGE_GENERIC_GAMEPAD:
			HandleRawInputGamepad(hDevice, hid);
			break;
		}
	}

	void HandleRawInputGamepad(HANDLE hDevice, const RAWHID& hid)
	{
		/*UINT preparseBufferSize;
		GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, NULL, &preparseBufferSize);
		if (preparseBufferSize == 0)
		{
			LogError("HandleRawInputHID: failed to parse HID data\n");
			return;
		}

		PHIDP_PREPARSED_DATA preparsedData = (PHIDP_PREPARSED_DATA)m_allocator.Allocate(preparseBufferSize);
		if (GetRawInputDeviceInfo(hDevice, RIDI_PREPARSEDDATA, preparsedData, &preparseBufferSize) <= 0)
		{
			LogError("HandleRawInputHID: failed to parse HID data\n");
		}

		bool buttonState[256] = { false };

		HIDP_CAPS caps;
		HidP_GetCaps(preparsedData, &caps);
		//https://www.codeproject.com/Articles/185522/Using-the-Raw-Input-API-to-Process-Joystick-Input

		USHORT buttonCapsLength = caps.NumberInputButtonCaps;//buttons?
		PHIDP_BUTTON_CAPS buttonCaps = (PHIDP_BUTTON_CAPS)m_allocator.Allocate(buttonCapsLength * sizeof(HIDP_BUTTON_CAPS));
		HidP_GetButtonCaps(HidP_Input, buttonCaps, &buttonCapsLength, preparsedData);

		USHORT numberOfButtons = buttonCaps->Range.UsageMax - buttonCaps->Range.UsageMin + 1;
		ULONG usageLength = (ULONG)numberOfButtons;
		PUSAGE usage = (PUSAGE)m_allocator.Allocate(numberOfButtons * sizeof(USAGE));
		HidP_GetUsages(HidP_Input, buttonCaps->UsagePage, 0, usage, &usageLength, preparsedData, (PCHAR)hid.bRawData, hid.dwSizeHid);


		USHORT valuesLength = caps.NumberInputValueCaps;//axes?
		PHIDP_VALUE_CAPS valuesCaps = (PHIDP_VALUE_CAPS)m_allocator.Allocate(valuesLength * sizeof(HIDP_VALUE_CAPS));
		HidP_GetValueCaps(HidP_Input, valuesCaps, &valuesLength, preparsedData);

		for (unsigned i = 0; i < caps.NumberInputValueCaps; ++i)
		{
			ULONG value;
			HidP_GetUsageValue(HidP_Input, valuesCaps[i].UsagePage, 0, valuesCaps[i].Range.UsageMin, &value, preparsedData, (PCHAR)hid.bRawData, hid.dwSizeHid);

			//switch (pValueCaps[i].Range.UsageMin)
		}

		m_allocator.Deallocate(buttonCaps);
		m_allocator.Deallocate(preparsedData);*/
	}

	void HandleRawInput(LPARAM lParam)
	{
		RAWINPUT raw;
		UINT size = sizeof(RAWINPUT);

		if(GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUTHEADER)) == (UINT)-1)
		{
			Log(LogType::Error, "HandleRawInput(): error happened in GetRawInputData()");
			return;
		}

		switch (raw.header.dwType)
		{
		case RIM_TYPEMOUSE:
			ASSERT((raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == 0);
			HandleRawInputMouse(raw.header.hDevice, raw.data.mouse);
			break;
		case RIM_TYPEKEYBOARD:
			HandleRawInputKeyboard(raw.header.hDevice, raw.data.keyboard);
			break;
		case RIM_TYPEHID:
			HandleRawInputHID(raw.header.hDevice, raw.data.hid);
			break;
		default:
			ASSERT2(false, "Unknown type of raw input device");
			break;
		}
	}

	void HandleRawInputDeviceChange(WPARAM wparam, LPARAM lparam)
	{
		HANDLE deviceHandle = (void*)lparam;

		RID_DEVICE_INFO deviceInfo;
		UINT size = sizeof(RID_DEVICE_INFO);
		if (GetRawInputDeviceInfo(deviceHandle, RIDI_DEVICEINFO, &deviceInfo, &size) > 0)
		{
			InputDeviceCategory category = InputDeviceCategory::None;
			switch (deviceInfo.dwType)
			{
			case RIM_TYPEMOUSE:
				category = InputDeviceCategory::Mouse;
				break;
			case RIM_TYPEKEYBOARD:
				category = InputDeviceCategory::Keyboard;
				break;
			case RIM_TYPEHID:
			{
				USHORT usage = GetHIDType(deviceHandle);
				switch (usage)
				{
				case HID_USAGE_GENERIC_GAMEPAD:
					category = InputDeviceCategory::Gamepad;
					break;
				}
			}
			break;
			}

			if (category == InputDeviceCategory::None)
				return;

			if (wparam == GIDC_ARRIVAL)
			{
				String deviceName(m_allocator);
				GetNameOfDevice(deviceHandle, deviceName);

				m_editor->RegisterDevice(lparam, category, deviceName);
			}
			else
				m_editor->UnregisterDevice(lparam);
		}
		else
		{
			Log(LogType::Error, "HandleRawInputDeviceChange: failed\n");
		}
	}

	USHORT GetHIDType(HANDLE deviceHandle)
	{
		UINT infoSize;
		GetRawInputDeviceInfo(deviceHandle, RIDI_DEVICEINFO, NULL, &infoSize);
		if (infoSize == 0)
			return 0;

		RID_DEVICE_INFO info;
		if (GetRawInputDeviceInfo(deviceHandle, RIDI_DEVICEINFO, &info, &infoSize) <= 0)
		{
			Log(LogType::Error, "GetHIDType: failed to get device type\n");
			return 0;
		}
		ASSERT(info.dwType == RIM_TYPEHID);
		return info.hid.usUsage;
	}

	void GetNameOfDevice(HANDLE deviceHandle, String& deviceName)
	{
		UINT registryPathSize;
		GetRawInputDeviceInfo(deviceHandle, RIDI_DEVICENAME, NULL, &registryPathSize);
		if (registryPathSize == 0)
			return;

		Array<char> rawRegistrypath(m_allocator);
		rawRegistrypath.Resize(registryPathSize);
		if (GetRawInputDeviceInfo(deviceHandle, RIDI_DEVICENAME, &rawRegistrypath[0], &registryPathSize) <= 0)
		{
			Log(LogType::Error, "RegisterRawInputDevices: failed to get device name\n");
			return;
		}

		if (rawRegistrypath[7] != '#')
		{
			deviceName.Set("Unknown device");
			return;
		}

		String registryPathStr(m_allocator, "SYSTEM\\CurrentControlSet\\Enum\\HID\\");
		unsigned startIdx = 8;
		unsigned endIdx = 8;
		for (; rawRegistrypath[endIdx] != '#'; ++endIdx);
		registryPathStr.Cat(&rawRegistrypath[startIdx], endIdx - startIdx);
		registryPathStr.Cat("\\");
		startIdx = endIdx + 1;
		endIdx = startIdx;
		for (; rawRegistrypath[endIdx] != '#'; ++endIdx);
		registryPathStr.Cat(&rawRegistrypath[startIdx], endIdx - startIdx);
		
		HKEY regKey;
		RegOpenKeyEx(HKEY_LOCAL_MACHINE, registryPathStr.Cstr(), 0, KEY_READ, &regKey);
		char entry[256] = { 0 };
		DWORD entryLength = 256;
		RegQueryValueEx(regKey, "DeviceDesc", NULL, NULL, (LPBYTE)entry, &entryLength);
		RegCloseKey(regKey);

		startIdx = entryLength - 1;
		for (; entry[startIdx] != ';' || startIdx == 0; --startIdx);
		++startIdx;
		deviceName.Set(&entry[startIdx], entryLength - startIdx);
	}

	void HandleMouseMove(LPARAM lparam)
	{
		WORD xPos = LOWORD(lparam);
		WORD yPos = HIWORD(lparam);
		m_editor->MouseMove(xPos, yPos);
	}

	void HandleResize(HWND hwnd, LPARAM lparam)
	{
		WORD width = LOWORD(lparam);
		WORD height = HIWORD(lparam);
		m_editor->Resize((windowHandle)hwnd, width, height);
	}

	LRESULT OnMessage(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		switch(msg)
		{
			case WM_KILLFOCUS:
				m_editor->SetFocus(false);
				break;
			case WM_SETFOCUS:
				m_editor->SetFocus(true);
				break;
			case WM_CLOSE:
				PostQuitMessage(0);
				break;
			case WM_MOVE:
				break;
			case WM_SIZE:
				HandleResize(hwnd, lparam);
				break;
			case WM_QUIT:
				m_finished = true;
				break;
			case WM_INPUT:
				HandleRawInput(lparam);
				break;
			case WM_MOUSEMOVE:
				HandleMouseMove(lparam);
				break;
			case WM_INPUT_DEVICE_CHANGE:
				HandleRawInputDeviceChange(wparam, lparam);
				break;
			/*case WM_SYSCOMMAND:
				if(wparam == SC_KEYMENU) //Remove beeping sound when ALT + some key is pressed.
					return 0;
				break;*/
		}
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	static LRESULT CALLBACK MsgProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		ASSERT(s_instance != nullptr);

		return s_instance->OnMessage(hwnd, msg, wparam, lparam);
	}


private:
	MainAllocator m_mainAllocator;
	HeapAllocator m_allocator;
	HWND m_hwnd = nullptr;

	Editor* m_editor = nullptr;

	static const int MAX_SUB_WINDOWS = 8;
	int m_subHwndSize = 0;
	HWND m_subHwnd[MAX_SUB_WINDOWS] = { nullptr };

	int m_exitCode = 0;
	bool m_finished = false;
	bool m_windowMode = true;

	static const char* WINDOW_CLASS_NAME;
	static AppImpl* s_instance;
};


const char* AppImpl::WINDOW_CLASS_NAME = "NewEngine";
AppImpl* AppImpl::s_instance = nullptr;


}


INT WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, INT)
{
	Veng::AppImpl app;
	app.Init();
	app.Run();
	app.Deinit();
	return app.GetExitCode();
}