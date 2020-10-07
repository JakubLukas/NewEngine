#pragma once

#include "core/int.h"
#include "core/allocator.h"
#include "core/math/vector.h"

#include "core/input/devices/input_device_mouse.h"
#include "core/input/devices/input_device_keyboard.h"
#include "core/input/devices/input_device_gamepad.h"


namespace Veng
{

template<class Type> class Array;
class String;


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

	InputEvent();
	InputEvent(const InputEvent& other);

	InputEvent& operator=(const InputEvent& other);


	Type type;
	float time;
	inputDeviceID deviceId;
	InputDeviceCategory deviceCategory;
	union
	{
		MouseDevice::Button mbCode;
		MouseDevice::Axis maCode;
		KeyboardDevice::Button kbCode;
		GamepadDevice::Button gbCode;
		GamepadDevice::Axis gaCode;
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
	static InputSystem* Create(Allocator& allocator);
	static void Destroy(InputSystem* system, Allocator& allocator);

public:
	virtual ~InputSystem() {}

	virtual void Enable(bool enable) = 0;

	virtual inputDeviceID RegisterDevice(inputDeviceHandle handle, InputDeviceCategory category, const String& name) = 0;
	virtual void UnregisterDevice(inputDeviceHandle handle) = 0;

	virtual void RegisterButtonEvent(inputDeviceHandle handle, KeyboardDevice::Button buttonId, bool pressed) = 0;
	virtual void RegisterButtonEvent(inputDeviceHandle handle, MouseDevice::Button buttonId, bool pressed) = 0;
	virtual void RegisterButtonEvent(inputDeviceHandle handle, GamepadDevice::Button buttonId, bool pressed) = 0;
	virtual void RegisterAxisEvent(inputDeviceHandle handle, MouseDevice::Axis axisId, const Vector3& delta) = 0;
	virtual void RegisterAxisEvent(inputDeviceHandle handle, GamepadDevice::Axis axisId, const Vector3& delta) = 0;

	virtual void Update(float deltaTime) = 0;

	virtual void LockCursor(bool lock) = 0;
	virtual void ShowCursor(bool show) = 0;

	virtual bool IsDeviceActive(inputDeviceID id) const = 0;
	virtual const Array<InputEvent>& GetInputEventBuffer() const = 0;
};


}