#pragma once


namespace Veng
{


struct WindowSize
{
	int width;
	int height;
};

class App
{
public:
	virtual ~App() {}

	virtual void* CreateSubWindow() = 0;
	virtual bool DestroySubWindow(void* hwnd) = 0;

	virtual void* GetMainWindowHandle() const = 0;
	virtual WindowSize GetWindowSize(void* hwnd) const = 0;

};

}