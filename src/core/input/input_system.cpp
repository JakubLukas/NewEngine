#include "input_system.h"


namespace Veng
{


class InputSystemImpl : public InputSystem
{
public:
	InputSystemImpl(IAllocator& allocator)
		: m_allocator(allocator)
		, m_devices(allocator)
		, m_events(allocator)
	{

	}


	inputDeviceID RegisterDevice(inputDeviceHandle handle, InputDevice::Category category) override
	{
		inputDeviceID dID = static_cast<inputDeviceID>(handle);
		InputDevice device(dID, category);
		m_devices.Push(device);

		return dID;
	}


	void UnregisterDevice(inputDeviceHandle id) override
	{
		for (unsigned i = 0; i < m_devices.Size(); ++i)
		{
			if (m_devices[i].GetDeviceId() == id)
			{
				m_devices.Erase(i);
				return;
			}
		}
	}


	bool RegisterKeyboardButtonEvent(inputDeviceHandle handle, KeyboardDevice::Button buttonId, bool pressed) override
	{
		return false;
	}
	bool RegisterMouseButtonEvent(inputDeviceHandle handle, MouseDevice::Button buttonId, bool pressed) override
	{
		return false;
	}
	bool RegisterMouseAxisEvent(inputDeviceHandle handle, MouseDevice::Axis axisId, const Vector3& delta) override
	{
		return false;
	}


	const Array<InputEvent>& GetInputEventBuffer() const override
	{
		return m_events;
	}

private:
	IAllocator& m_allocator;
	Array<InputDevice> m_devices;
	Array<InputEvent> m_events;
};


InputSystem* InputSystem::Create(IAllocator& allocator)
{
	void* mem = allocator.Allocate(sizeof(InputSystemImpl));
	return (InputSystem*)(new (NewPlaceholder(), mem) InputSystemImpl(allocator));
}

void InputSystem::Destroy(InputSystem* inputSystem, IAllocator& allocator)
{
	InputSystemImpl* p = (InputSystemImpl*)inputSystem;
	p->~InputSystemImpl();
	allocator.Deallocate(p);
}


}