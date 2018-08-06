#pragma once

#include "core/int.h"


namespace Veng
{

typedef void* windowHandle;
static const windowHandle INVALID_WINDOW_HANDLE = nullptr;

struct WindowMetrics
{
	u32 x;
	u32 y;
};


class App
{
public:
	virtual ~App() {}

	virtual windowHandle CreateSubWindow() = 0;
	virtual windowHandle CreateSubWindow(WindowMetrics position, WindowMetrics size) = 0;
	virtual bool DestroySubWindow(void* hwnd) = 0;

	virtual void DockSubWindow(windowHandle hwnd) = 0;
	virtual void UndockSubWindow(windowHandle hwnd) = 0;

	virtual windowHandle GetMainWindowHandle() const = 0;
	virtual WindowMetrics GetWindowSize(windowHandle hwnd) const = 0;

};

}