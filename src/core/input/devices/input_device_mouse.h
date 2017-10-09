#pragma once

#include "core/input/input_system.h"


namespace Veng
{


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


}