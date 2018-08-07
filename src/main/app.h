#pragma once

#include "core/int.h"


namespace Veng
{

typedef void* windowHandle;
static const windowHandle INVALID_WINDOW_HANDLE = nullptr;

struct WindowRect
{
	u32 positionX;
	u32 positionY;
	u32 width;
	u32 height;
};

struct WindowSize
{
	u32 x;
	u32 y;
};


class App
{
public:
	virtual ~App() {}

	virtual windowHandle CreateSubWindow() = 0;
	virtual windowHandle CreateSubWindow(WindowRect size) = 0;
	virtual bool DestroySubWindow(void* hwnd) = 0;

	virtual void DockSubWindow(windowHandle hwnd) = 0;
	virtual void UndockSubWindow(windowHandle hwnd) = 0;

	virtual WindowSize GetWindowPosition(windowHandle hwnd) const = 0;
	virtual void SetWindowPosition(windowHandle hwnd, WindowSize pos) = 0;

	virtual WindowSize GetWindowSize(windowHandle hwnd) const = 0;
	virtual void SetWindowSize(windowHandle hwnd, WindowSize size) = 0;

	virtual windowHandle GetMainWindowHandle() const = 0;

};

}