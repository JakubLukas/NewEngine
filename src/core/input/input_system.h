#pragma once

#include "core/int.h"
#include "core/allocators.h"
#include "core/vector3.h"
#include "core/associative_array.h"


namespace Veng
{

typedef u8 deviceID;
typedef void* deviceHandle;

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


	InputDevice(deviceID deviceId, Category category)
		: m_deviceId(deviceId)
		, m_category(category)
		, m_active(false)
	{
	}

	Category GetCategory() const { return m_category; }
	
	deviceID GetDeviceId() const { return m_deviceId; }

	bool IsActive() const { return m_active; };

	virtual u8 GetButtonsCount() const { return 0; }

	virtual u8 GetAxisCount() const { return 0; };

protected:
	Category m_category;
	bool m_active;
	deviceID m_deviceId;
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
	static InputSystem* CreateInputSystem(IAllocator& allocator);
	static void DestroyInputSystem(InputSystem* inputSystem);

	virtual ~InputSystem() = 0;

	virtual deviceID RegisterDevice(deviceHandle handle, InputDevice::Category category) = 0;
	virtual void UnregisterDevice(deviceID id) = 0;

	virtual const InputEvent* GetInputEventBuffer() const = 0;
	virtual unsigned GetInputEventBufferSize() const = 0;

	//push win events
};


}