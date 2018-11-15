#pragma once


namespace Veng
{


class SpinLock
{
public:
	SpinLock();
	SpinLock(SpinLock&) = delete;
	SpinLock(SpinLock&&) = delete;
	SpinLock& operator=(SpinLock&) = delete;
	SpinLock& operator=(SpinLock&&) = delete;
	~SpinLock();

	void Lock();
	void Unlock();

private:
	volatile long m_lock;
};


template<class LockType>
class ScopeLock
{
public:
	ScopeLock(LockType& lock);
	ScopeLock(ScopeLock&) = delete;
	ScopeLock(ScopeLock&&) = delete;
	ScopeLock& operator=(ScopeLock&) = delete;
	ScopeLock& operator=(ScopeLock&&) = delete;
	~ScopeLock();

private:
	LockType& m_lock;
};


template<class LockType>
ScopeLock<LockType>::ScopeLock(LockType& lock)
	: m_lock(lock)
{
	m_lock.Lock();
}

template<class LockType>
ScopeLock<LockType>::~ScopeLock()
{
	m_lock.Unlock();
}


}