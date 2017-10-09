#include "input_system.h"


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