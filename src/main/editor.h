#pragma once

#include "core/int.h"
#include "core/input/input_system.h"
#include "app.h"


namespace Veng
{

class Allocator;
class Engine;


namespace Editor
{


class EditorApp
{
public:
	static EditorApp* Create(Allocator& allocator, App& app);
	static void Destroy(EditorApp* editor, Allocator& allocator);

public:
	virtual ~EditorApp() {}


	virtual void Init() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Deinit() = 0;

	virtual void Resize(windowHandle handle, i32 width, i32 height) = 0;
	virtual void SetFocus(windowHandle handle, bool hasFocus) = 0;


	virtual inputDeviceID RegisterDevice(inputDeviceHandle deviceHandle, InputDeviceCategory category, const String& name) = 0;
	virtual void UnregisterDevice(inputDeviceHandle deviceHandle) = 0;

	virtual void RegisterButtonEvent(windowHandle handle, inputDeviceHandle deviceHandle, KeyboardDevice::Button buttonId, bool pressed) = 0;
	virtual void RegisterButtonEvent(windowHandle handle, inputDeviceHandle deviceHandle, MouseDevice::Button buttonId, bool pressed) = 0;
	virtual void RegisterButtonEvent(windowHandle handle, inputDeviceHandle deviceHandle, GamepadDevice::Button buttonId, bool pressed) = 0;
	virtual void RegisterAxisEvent(windowHandle handle, inputDeviceHandle deviceHandle, MouseDevice::Axis axisId, const Vector3& delta) = 0;
	virtual void RegisterAxisEvent(windowHandle handle, inputDeviceHandle deviceHandle, GamepadDevice::Axis axisId, const Vector3& delta) = 0;

	virtual void MouseMove(i32 xPos, i32 yPos) = 0;
	virtual void InputChar(u8 character) = 0;
};


}


}