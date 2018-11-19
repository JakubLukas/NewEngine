#include "threads.h"

#include "os_utils.h"


namespace Veng
{

const i32 FREE = 0;
const i32 LOCKED = 1;


SpinLock::SpinLock()
	: m_lock(FREE)
{}

SpinLock::~SpinLock()
{}

void SpinLock::Lock()
{
	for (;;)
	{
		if (m_lock == FREE && AtomicCompareExchange(&m_lock, LOCKED, FREE) == FREE)
			break;
		CpuRelax();
	}
}

void SpinLock::Unlock()
{
	m_lock = FREE;
}


}
