#pragma once

#include "core/int.h"
#include "core/input/input_system.h"
#include "app.h"


namespace Veng
{

class IAllocator;
class Engine;


class Editor
{
public:
	static Editor* Create(IAllocator& allocator, App& app);
	static void Destroy(Editor* editor, IAllocator& allocator);

public:
	virtual ~Editor() {}


	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Deinit() = 0;

	virtual void Resize(windowHandle handle, u32 width, u32 height) = 0;
	virtual void SetFocus(bool hasFocus) = 0;


	virtual inputDeviceID RegisterDevice(inputDeviceHandle handle, InputDeviceCategory category, const String& name) = 0;
	virtual void UnregisterDevice(inputDeviceHandle handle) = 0;

	virtual void RegisterButtonEvent(inputDeviceHandle handle, KeyboardDevice::Button buttonId, bool pressed) = 0;
	virtual void RegisterButtonEvent(inputDeviceHandle handle, MouseDevice::Button buttonId, bool pressed) = 0;
	virtual void RegisterButtonEvent(inputDeviceHandle handle, GamepadDevice::Button buttonId, bool pressed) = 0;
	virtual void RegisterAxisEvent(inputDeviceHandle handle, MouseDevice::Axis axisId, const Vector3& delta) = 0;
	virtual void RegisterAxisEvent(inputDeviceHandle handle, GamepadDevice::Axis axisId, const Vector3& delta) = 0;

	//virtual Engine* GetEngine() = 0;//////////// tmp function

};

}