#pragma once

#include "core/int.h"


namespace Veng
{

typedef void* windowHandle;
static const windowHandle INVALID_WINDOW_HANDLE = nullptr;

struct WindowSize
{
	u32 width;
	u32 height;
};


class App
{
public:
	virtual ~App() {}

	virtual windowHandle CreateSubWindow() = 0;
	virtual windowHandle CreateSubWindow(WindowSize size) = 0;
	virtual bool DestroySubWindow(void* hwnd) = 0;

	virtual void DockSubWindow(windowHandle hwnd) = 0;
	virtual void UndockSubWindow(windowHandle hwnd) = 0;

	virtual windowHandle GetMainWindowHandle() const = 0;
	virtual WindowSize GetWindowSize(windowHandle hwnd) const = 0;

};

}