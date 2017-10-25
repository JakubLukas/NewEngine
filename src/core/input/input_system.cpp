#include "input_system.h"

#include "core/associative_array.h"


namespace Veng
{


struct InputDevice
{
	static const int MAX_NAME_LENGTH = 128;

	InputDevice()
		: category(InputDeviceCategory::None)
		, id(0)
		, active(false)
	{
	}


	InputDevice(inputDeviceID deviceId, InputDeviceCategory category)
		: category(category)
		, id(deviceId)
		, active(false)
	{
	}

	InputDevice(const InputDevice& other)
		: category(other.category)
		, id(other.id)
		, active(other.active)
	{
	}


	InputDeviceCategory category;
	inputDeviceID id;
	bool active;
	char name[MAX_NAME_LENGTH] = { 0 };
};



class InputSystemImpl : public InputSystem
{
public:
	InputSystemImpl(IAllocator& allocator)
		: m_allocator(allocator)
		, m_devices(allocator)
		, m_events(allocator)
	{

	}


	inputDeviceID RegisterDevice(inputDeviceHandle handle, InputDeviceCategory category, const String& name) override
	{
		inputDeviceID dID = static_cast<inputDeviceID>(handle);
		InputDevice device(dID, category);
		device.active = true;
		m_devices.Insert(handle, device);
		unsigned strLen = (InputDevice::MAX_NAME_LENGTH < name.Length()) ? InputDevice::MAX_NAME_LENGTH : name.Length();
		StrCopy(device.name, name.Cstr(), strLen);

		return dID;
	}


	void UnregisterDevice(inputDeviceHandle handle) override
	{
		/*for (unsigned i = 0; i < m_devices.Size(); ++i)
		{
			if (m_devices[i].GetDeviceId() == id)
			{
				m_devices.Erase(i);
				return;
			}
		}*/
		InputDevice* device;
		if (m_devices.Find(handle, device))
		{
			device->active = false;
		}
		else
		{
			ASSERT2(false, "Tried to unregister unknown device");
		}
	}


	bool RegisterButtonEvent(inputDeviceHandle handle, KeyboardDevice::Button buttonId, bool pressed) override
	{
		InputDevice* device;
		if (m_devices.Find(handle, device))
		{
			InputEvent newEvent;
			newEvent.type = InputEvent::Type::ButtonChanged;
			newEvent.time = 0.0f;
			newEvent.deviceId = device->id;
			newEvent.deviceCategory = device->category;
			newEvent.kbCode = buttonId;
			newEvent.pressed = pressed;
			m_events.Push(newEvent);
			return true;
		}
		return false;
	}

	bool RegisterButtonEvent(inputDeviceHandle handle, MouseDevice::Button buttonId, bool pressed) override
	{
		InputDevice* device;
		if (m_devices.Find(handle, device))
		{
			InputEvent newEvent;
			newEvent.type = InputEvent::Type::ButtonChanged;
			newEvent.time = 0.0f;
			newEvent.deviceId = device->id;
			newEvent.deviceCategory = device->category;
			newEvent.mbCode = buttonId;
			newEvent.pressed = pressed;
			m_events.Push(newEvent);
			return true;
		}
		return false;
	}

	bool RegisterButtonEvent(inputDeviceHandle handle, GamepadDevice::Button buttonId, bool pressed) override
	{
		InputDevice* device;
		if (m_devices.Find(handle, device))
		{
			InputEvent newEvent;
			newEvent.type = InputEvent::Type::ButtonChanged;
			newEvent.time = 0.0f;
			newEvent.deviceId = device->id;
			newEvent.deviceCategory = device->category;
			newEvent.gbCode = buttonId;
			newEvent.pressed = pressed;
			m_events.Push(newEvent);
			return true;
		}
		return false;
	}

	bool RegisterAxisEvent(inputDeviceHandle handle, MouseDevice::Axis axisId, const Vector3& delta) override
	{
		InputDevice* device;
		if (m_devices.Find(handle, device))
		{
			InputEvent newEvent;
			newEvent.type = InputEvent::Type::AxisChanged;
			newEvent.time = 0.0f;
			newEvent.deviceId = device->id;
			newEvent.deviceCategory = device->category;
			newEvent.maCode = axisId;
			newEvent.axis = delta;
			m_events.Push(newEvent);
			return true;
		}
		return false;
	}

	bool RegisterAxisEvent(inputDeviceHandle handle, GamepadDevice::Axis axisId, const Vector3& delta) override
	{
		InputDevice* device;
		if (m_devices.Find(handle, device))
		{
			InputEvent newEvent;
			newEvent.type = InputEvent::Type::AxisChanged;
			newEvent.time = 0.0f;
			newEvent.deviceId = device->id;
			newEvent.deviceCategory = device->category;
			newEvent.gaCode = axisId;
			newEvent.axis = delta;
			m_events.Push(newEvent);
			return true;
		}
		return false;
	}


	void Update(float deltaTime) override
	{
		m_events.Clear();
	}


	bool IsDeviceActive(inputDeviceID id) const override
	{
		for (const InputDevice& device : m_devices)
		{
			if (device.id == id)
				return device.active;
		}
		ASSERT2(id, "Device with used id doesn't exist");
		return false;
	}


	const Array<InputEvent>& GetInputEventBuffer() const override
	{
		return m_events;
	}

private:
	IAllocator& m_allocator;
	AssociativeArray<inputDeviceHandle, InputDevice> m_devices;
	Array<InputEvent> m_events;
};


InputSystem* InputSystem::Create(IAllocator& allocator)
{
	return (InputSystem*)(new (NewPlaceholder(), allocator) InputSystemImpl(allocator));
}

void InputSystem::Destroy(InputSystem* system, IAllocator& allocator)
{
	InputSystemImpl* p = (InputSystemImpl*)system;
	p->~InputSystemImpl();
	allocator.Deallocate(p);
}


}