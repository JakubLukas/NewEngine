#include "threads.h"

#include <windows.h>


namespace Veng
{

const long FREE = 0;
const long LOCKED = 1;


SpinLock::SpinLock()
	: m_lock(FREE)
{}

SpinLock::~SpinLock()
{}

void SpinLock::Lock()
{
	while (InterlockedCompareExchange(&m_lock, LOCKED, FREE) != FREE)
		;
}

void SpinLock::Unlock()
{
	m_lock = FREE;
}


}
