#include "input_system.h"

#include "devices/input_device_mouse.h"
#include "devices/input_device_keyboard.h"


namespace Veng
{


class InputSystemImpl : public InputSystem
{
public:
	~InputSystemImpl() override
	{

	}


	deviceID RegisterDevice(deviceHandle handle, InputDevice::Category category) override
	{
		switch(category)
		{
			case InputDevice::Category::MOUSE:
				MouseDevice device()
				break;
			case InputDevice::Category::KEYBOARD:
				break;
			case InputDevice::Category::GAMEPAD:
				break;
			default:
				break;
		}
		//m_devices.Insert(handle, )
	}


	void UnregisterDevice(deviceID id) override
	{

	}


	const InputEvent* GetInputEventBuffer() const override
	{

	}


	unsigned GetInputEventBufferSize() const override
	{

	}

private:
	IAllocator& m_allocator;
	AssociativeArray<deviceHandle, InputDevice> m_devices;
};


InputSystem* InputSystem::CreateInputSystem(IAllocator& allocator)
{
	return nullptr;
}

void InputSystem::DestroyInputSystem(InputSystem* inputSystem)
{

}


}