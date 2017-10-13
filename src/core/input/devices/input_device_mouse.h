#pragma once

#include "core/int.h"


namespace Veng
{


namespace MouseDevice
{


enum class Button : u8
{
	Left,
	Right,
	Middle,
	Extra4,
	Extra5
};


enum class Axis : u8
{
	Wheel,
	Movement
};


};


}