#include "event_queue.h"

#include "core/iallocator.h"
#include "core/asserts.h"
#include "core/memory.h"


namespace Veng
{

namespace Editor
{


EventQueue::EventQueue(IAllocator& allocator)
	: m_allocator(allocator)
{}

EventQueue::~EventQueue()
{
	if (m_pushBuffer != nullptr || m_pullBuffer != nullptr)
	{
		if (m_pushBuffer < m_pullBuffer)
			m_allocator.Deallocate(m_pushBuffer);
		else
			m_allocator.Deallocate(m_pullBuffer);
	}
}


void EventQueue::FrameUpdate()
{
	m_pullSize = 0;
	u8* pos = (u8*)m_pushBuffer;
	while (pos < m_pushPosition)
	{
		m_pullEvents[m_pullSize] = (Event*)pos;
		m_pullSize++;
		ASSERT(((Event*)pos)->size > 0);
		pos = pos + ((Event*)pos)->size;
	}

	void* oldBuffer = m_pullBuffer;
	m_pullBuffer = m_pushBuffer;
	m_pushBuffer = oldBuffer;
	m_pushPosition = (u8*)m_pushBuffer;
}


void EventQueue::PushEvent(Event& event)
{
	if (m_pushPosition + event.size >= (u8*)m_pushBuffer + m_capacity * sizeof(Event))
		Enlarge();

	memory::Copy(m_pushPosition, &event, event.size);
	m_pushPosition += event.size;
}

Event** EventQueue::PullEvents() const { return m_pullEvents; }

size_t EventQueue::GetPullEventsSize() const { return m_pullSize; }


void EventQueue::Enlarge()
{
	size_t oldCapacity = m_capacity;
	void* oldPushBuffer = m_pushBuffer;
	void* oldPullBuffer = m_pullBuffer;
	Event** oldPullEvents = m_pullEvents;
	
	m_capacity = (m_capacity == 0) ? INITIAL_SIZE : m_capacity * ENLARGE_MULTIPLIER;

	void* newBuffers = m_allocator.Allocate(m_capacity * 2 * sizeof(Event) + m_capacity * sizeof(Event*), alignof(Event));
	m_pushBuffer = newBuffers;
	m_pullBuffer = (u8*)newBuffers + m_capacity * sizeof(Event);
	m_pullEvents = (Event**)((u8*)m_pullBuffer + m_capacity * sizeof(Event));
	m_pushPosition = (u8*)m_pushBuffer + ((uintptr)m_pushPosition - (uintptr)oldPushBuffer);

	memory::Copy(m_pushBuffer, oldPushBuffer, oldCapacity * sizeof(Event));
	memory::Copy(m_pullBuffer, oldPullBuffer, oldCapacity * sizeof(Event));
	memory::Copy(m_pullEvents, oldPullEvents, oldCapacity * sizeof(Event*));

	if (oldPushBuffer != nullptr || oldPullBuffer != nullptr)
	{
		if (oldPushBuffer < oldPullBuffer)
			m_allocator.Deallocate(oldPushBuffer);
		else
			m_allocator.Deallocate(oldPullBuffer);
	}
}


}

}
