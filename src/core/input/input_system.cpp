#include "input_system.h"

#include "core/containers/associative_array.h"
#include "core/memory.h"
#include "core/containers/array.h"
#include "core/string.h"

#include "core/os/os_utils.h"


namespace Veng
{


struct MousePos
{
	int x;
	int y;
};


InputEvent::InputEvent()
{

}


InputEvent::InputEvent(const InputEvent& other)
{
	memory::Copy(this, &other, sizeof(InputEvent));
}


InputEvent& InputEvent::operator=(const InputEvent& other)
{
	memory::Copy(this, &other, sizeof(InputEvent));
	return *this;
}


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

	InputDevice& operator=(const InputDevice& other)
	{
		memory::Copy(this, &other, sizeof(InputDevice));
		return *this;
	}


	InputDeviceCategory category;
	inputDeviceID id;
	bool active;
	char name[MAX_NAME_LENGTH] = { 0 };
};



class InputSystemImpl : public InputSystem
{
public:
	InputSystemImpl(Allocator& allocator)
		: m_allocator(allocator)
		, m_devices(allocator)
		, m_events(allocator)
	{}


	~InputSystemImpl() override
	{}


	void Enable(bool enable) override
	{
		m_active = enable;
	}


	inputDeviceID RegisterDevice(inputDeviceHandle handle, InputDeviceCategory category, const String& name) override
	{
		inputDeviceID dID = static_cast<inputDeviceID>(handle);
		InputDevice device(dID, category);
		device.active = true;
		m_devices.Insert(handle, device);
		size_t strLen = (InputDevice::MAX_NAME_LENGTH < name.Length()) ? InputDevice::MAX_NAME_LENGTH : name.Length();
		string::Copy(device.name, name.Cstr(), strLen);

		return dID;
	}


	void UnregisterDevice(inputDeviceHandle handle) override
	{
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


	void RegisterButtonEvent(inputDeviceHandle handle, KeyboardDevice::Button buttonId, bool pressed) override
	{
		if (!m_active)
			return;

		InputDevice* device;
		if (m_devices.Find(handle, device))
		{
			InputEvent newEvent;
			newEvent.type = InputEvent::Type::ButtonChanged;
			newEvent.time = 0.0f;////////////////////////////////////////////
			newEvent.deviceId = device->id;
			newEvent.deviceCategory = device->category;
			newEvent.kbCode = buttonId;
			newEvent.pressed = pressed;
			m_events.PushBack(newEvent);
		}
	}

	void RegisterButtonEvent(inputDeviceHandle handle, MouseDevice::Button buttonId, bool pressed) override
	{
		if (!m_active)
			return;

		InputDevice* device;
		if (m_devices.Find(handle, device))
		{
			InputEvent newEvent;
			newEvent.type = InputEvent::Type::ButtonChanged;
			newEvent.time = 0.0f;////////////////////////////////////////////
			newEvent.deviceId = device->id;
			newEvent.deviceCategory = device->category;
			newEvent.mbCode = buttonId;
			newEvent.pressed = pressed;
			m_events.PushBack(newEvent);
			//Log(LogType::Error, "mouse click %d")
		}
	}

	void RegisterButtonEvent(inputDeviceHandle handle, GamepadDevice::Button buttonId, bool pressed) override
	{
		if (!m_active)
			return;

		InputDevice* device;
		if (m_devices.Find(handle, device))
		{
			InputEvent newEvent;
			newEvent.type = InputEvent::Type::ButtonChanged;
			newEvent.time = 0.0f;////////////////////////////////////////////
			newEvent.deviceId = device->id;
			newEvent.deviceCategory = device->category;
			newEvent.gbCode = buttonId;
			newEvent.pressed = pressed;
			m_events.PushBack(newEvent);
		}
	}

	void RegisterAxisEvent(inputDeviceHandle handle, MouseDevice::Axis axisId, const Vector3& delta) override
	{
		if (!m_active)
			return;

		InputDevice* device;
		if (m_devices.Find(handle, device))
		{
			InputEvent newEvent;
			newEvent.type = InputEvent::Type::AxisChanged;
			newEvent.time = 0.0f;////////////////////////////////////////////
			newEvent.deviceId = device->id;
			newEvent.deviceCategory = device->category;
			newEvent.maCode = axisId;
			newEvent.axis = delta;
			m_events.PushBack(newEvent);
		}
	}

	void RegisterAxisEvent(inputDeviceHandle handle, GamepadDevice::Axis axisId, const Vector3& delta) override
	{
		if (!m_active)
			return;

		InputDevice* device;
		if (m_devices.Find(handle, device))
		{
			InputEvent newEvent;
			newEvent.type = InputEvent::Type::AxisChanged;
			newEvent.time = 0.0f;////////////////////////////////////////////
			newEvent.deviceId = device->id;
			newEvent.deviceCategory = device->category;
			newEvent.gaCode = axisId;
			newEvent.axis = delta;
			m_events.PushBack(newEvent);
		}
	}


	void Update(float deltaTime) override
	{
		m_events.Clear();
	}


	void LockCursor(bool lock) override
	{
		m_mouseLocked = lock;
		if (lock) {
			os::GetMouseCursorPos(m_mousePositon.x, m_mousePositon.y);
			os::ClipMouseCursor(m_mousePositon.x, m_mousePositon.y, 1, 1);
		}
		else {
			os::ClipMouseCursor(-1, -1, -1, -1);
		}
	}

	void ShowCursor(bool show) override
	{
		os::ShowMouseCursor(show);
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
	Allocator& m_allocator;
	AssociativeArray<inputDeviceHandle, InputDevice> m_devices;
	Array<InputEvent> m_events;
	MousePos m_mousePositon;
	bool m_mouseLocked = false;
	bool m_active = true;
};


InputSystem* InputSystem::Create(Allocator& allocator)
{
	return NEW_OBJECT(allocator, InputSystemImpl)(allocator);
}

void InputSystem::Destroy(InputSystem* system, Allocator& allocator)
{
	DELETE_OBJECT(allocator, system);
}


}