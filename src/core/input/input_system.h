#pragma once

#include "core/int.h"
#include "core/allocators.h"
#include "core/vector3.h"
#include "core/array.h"
#include "core/string.h"

#include "core/input/devices/input_device_mouse.h"
#include "core/input/devices/input_device_keyboard.h"


namespace Veng
{


typedef uintptr inputDeviceID;
typedef uintptr inputDeviceHandle;


enum class InputDeviceCategory : u8
{
	None,
	Mouse,
	Keyboard,
	Gamepad
};


struct InputEvent
{
	enum class Type : u8
	{
		DeviceActiveChanged,
		ButtonChanged,
		AxisChanged
	};

	InputEvent() {}

	Type type;
	float time;
	inputDeviceID deviceId;
	InputDeviceCategory deviceCategory;
	union
	{
		MouseDevice::Button mbCode;
		MouseDevice::Axis maCode;
		KeyboardDevice::Button kbCode;
	};
	union
	{
		bool connected;
		bool pressed;
		Vector3 axis;
	};
};


class InputSystem
{
public:
	static InputSystem* Create(IAllocator& allocator);
	static void Destroy(InputSystem* inputSystem, IAllocator& allocator);


	virtual bool IsDeviceActive(inputDeviceID id) const = 0;
	virtual const Array<InputEvent>& GetInputEventBuffer() const = 0;


	virtual inputDeviceID RegisterDevice(inputDeviceHandle handle, InputDeviceCategory category, const String& name) = 0;
	virtual void UnregisterDevice(inputDeviceHandle handle) = 0;

	virtual bool RegisterKeyboardButtonEvent(inputDeviceHandle handle, KeyboardDevice::Button buttonId, bool pressed) = 0;
	virtual bool RegisterMouseButtonEvent(inputDeviceHandle handle, MouseDevice::Button buttonId, bool pressed) = 0;
	virtual bool RegisterMouseAxisEvent(inputDeviceHandle handle, MouseDevice::Axis axisId, const Vector3& delta) = 0;
};


}