#include "color.h"


namespace Veng
{


Color::Color()
	: abgr(0xFF808080)
{}

Color::Color(u8 r, u8 g, u8 b, u8 a /* = 255 */)
	: abgr((a << 24) + (b << 16) + (g << 8) + r)
{}

Color::Color(u32 abgr)
	: abgr(abgr)
{}


}
