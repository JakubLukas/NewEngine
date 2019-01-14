#pragma once

#include "core/int.h"


namespace Veng
{


struct Color
{
	Color();
	Color(u8 r, u8 g, u8 b, u8 a = 255);
	explicit Color(u32 abgr);

	u32 abgr;
};


}