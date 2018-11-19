#include "../os_utils.h"

#include <core/os/win/simple_windows.h>


namespace Veng
{


void CpuRelax()
{
	YieldProcessor();
}


i32 AtomicCompareExchange(volatile i32* destination, const i32 exchange, const i32 comperand)
{
	return InterlockedCompareExchange(
		reinterpret_cast<volatile LONG*>(destination),
		exchange,
		comperand
	);
}


}
