#pragma once

#include "core/int.h"
#include "core/allocators.h"
#include "core/vector3.h"
#include "core/associative_array.h"


namespace Veng
{


/*
InputController Stingray functions

constructor(alloc, inputManager)
destructor
update(float)
flushState()
type
category
name
active
connected (right now)
disconnected(right now)
#buttons
getButton
anyButton
buttonName
buttonLocaleName
buttonId(string)
hasButton(string)
pressedMask ??
releasedMask ??
#axes
getAxis
deadZone
setDeadZone
axisName
axisId(string)
hasAxis(string)
*/

class InputDevice
{
public:
	enum class Type : u8
	{
		BUTTON,
		AXIS
	};

	enum class Category : u8
	{
		MOUSE,
		KEYBOARD,
		GAMEPAD
	};


	InputDevice(u8 deviceId, Category category)
		: m_deviceId(deviceId)
		, m_category(category)
		, m_active(false)
	{
	}

	Category GetCategory() const { return m_category; }
	
	u8 GetDeviceId() const { return m_deviceId; }

	bool IsActive() const { return m_active; };

	virtual u8 GetButtonsCount() const { return 0; }

	virtual u8 GetAxisCount() const { return 0; };

protected:
	Category m_category;
	bool m_active;
	u8 m_deviceId;
};



class MouseDevice : public InputDevice
{
public:
	enum class Button : u8
	{
		Left,
		Right,
		Middle,
		Extra1,
		Extra2,
		Extra3,
		Extra4,
		Extra5,
		Count
	};

	enum class Axis : u8
	{
		Wheel,
		Movement,
		Count
	};

	MouseDevice(u8 deviceId, Category category)
		: InputDevice(deviceId, category)
	{
		m_category = Category::MOUSE;
	}

	virtual u8 GetButtonsCount() const override { return (u8)Button::Count; }

	virtual u8 GetAxisCount() const override { return (u8)Axis::Count; };
};


struct InputEvent
{
	enum class Type : u8
	{
		DeviceConnect,
		DeviceDisconnect,
		ButtonPressed,
		ButtonReleased,
		AxisChanged
	};

	Type eventType;
	float time;
	const InputDevice& device;
	u8 item;//USBHID code
};


class InputSystem//make pure virtual interface
{
public:
	explicit InputSystem(IAllocator& allocator);

private:
	IAllocator& m_allocator;
	AssociativeArray<void*, InputDevice> m_devices;
};


}