#pragma once

#include "core/int.h"


namespace Veng
{


void CpuRelax();//TODO: function call is overkill


i32 AtomicCompareExchange(volatile i32* destination, const i32 exchange, const i32 comperand);


//other atomics


}
