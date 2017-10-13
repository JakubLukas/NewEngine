#pragma once

#include "core/int.h"


namespace Veng
{


namespace GamepadDevice
{


enum class Button : u8
{
	A,
	B,
	X,
	Y,
	ShoulderLeft,
	ShoulderRight,
	Menu,
	Options,
	StickLeft,
	StickRight
};


enum class Axis : u8
{
	StickLeft,
	StickRight,
	DPad,
	Triggers
};


};


}