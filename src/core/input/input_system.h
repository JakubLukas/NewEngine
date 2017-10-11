#pragma once

#include "core/int.h"
#include "core/allocators.h"
#include "core/vector3.h"
#include "core/array.h"

#include "core/input/devices/input_device_mouse.h"
#include "core/input/devices/input_device_keyboard.h"


namespace Veng
{


typedef uintptr inputDeviceID;
typedef uintptr inputDeviceHandle;


class InputDevice final
{
public:
	enum class Type : u8
	{
		Button,
		Axis
	};

	enum class Category : u8
	{
		None,
		Mouse,
		Keyboard,
		Gamepad
	};


	InputDevice()
		: m_deviceId(0)
		, m_category(Category::None)
		, m_active(false)
	{
	}


	InputDevice(inputDeviceID deviceId, Category category)
		: m_deviceId(deviceId)
		, m_category(category)
		, m_active(false)
	{
	}

	InputDevice(const InputDevice& other)
		: m_deviceId(other.m_deviceId)
		, m_category(other.m_category)
		, m_active(other.m_active)
	{
	}

	Category GetCategory() const { return m_category; }
	
	inputDeviceID GetDeviceId() const { return m_deviceId; }

	bool IsActive() const { return m_active; };
protected:
	Category m_category;
	bool m_active;
	inputDeviceID m_deviceId;
};


struct InputEvent
{
	enum class Type : u8
	{
		DeviceActiveChanged,
		ButtonChanged,
		AxisChanged
	};

	Type eventType;
	float time;
	const InputDevice& device;
	u8 code;
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


	virtual const Array<InputEvent>& GetInputEventBuffer() const = 0;


	virtual inputDeviceID RegisterDevice(inputDeviceHandle handle, InputDevice::Category category) = 0;
	virtual void UnregisterDevice(inputDeviceHandle id) = 0;

	virtual bool RegisterKeyboardButtonEvent(inputDeviceHandle handle, KeyboardDevice::Button buttonId, bool pressed) = 0;
	virtual bool RegisterMouseButtonEvent(inputDeviceHandle handle, MouseDevice::Button buttonId, bool pressed) = 0;
	virtual bool RegisterMouseAxisEvent(inputDeviceHandle handle, MouseDevice::Axis axisId, const Vector3& delta) = 0;
};


}