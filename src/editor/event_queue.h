#pragma once

#include "core/int.h"
#include "core/entity.h"
#include "core/resource/resource.h"
#include "core/input/devices/input_device_keyboard.h"
#include "core/input/devices/input_device_mouse.h"


namespace Veng
{

enum class worldId : u32;

class IAllocator;


namespace Editor
{

// ----------------------------------------------

enum EventType : u32
{
	SelectWorld,
	SelectEntity,
	SelectCamera,
	SelectResource,
	ChangedSize,
};

// ----------------------------------------------

struct Event
{
	Event() {}
	Event(EventType type, u32 size) : type(type), size(size) {}

	EventType type;
	u32 size;
};


struct EventSelectWorld : public Event
{
	EventSelectWorld() : Event(EventType::SelectWorld, sizeof(EventSelectWorld)) {}

	worldId id;
};

struct EventSelectEntity : public Event
{
	EventSelectEntity() : Event(EventType::SelectEntity, sizeof(EventSelectEntity)) {}

	Entity entity;
};

struct EventSelectCamera : public Event
{
	EventSelectCamera() : Event(EventType::SelectCamera, sizeof(EventSelectCamera)) {}

	Entity camera;
};

struct EventChangedSize : public Event
{
	EventChangedSize() : Event(EventType::ChangedSize, sizeof(EventChangedSize)) {}

	u32 width;
	u32 height;
};

struct EventSelectResource : public Event
{
	EventSelectResource() : Event(EventType::SelectResource, sizeof(EventSelectResource)) {}

	ResourceType type;
	resourceHandle resource;
};

// ----------------------------------------------

class EventQueue final
{
public:
	explicit EventQueue(IAllocator& allocator);
	~EventQueue();

	void FrameUpdate();

	void PushEvent(Event& event);

	Event** PullEvents() const;
	size_t GetPullEventsSize() const;


private:
	enum
	{
		INITIAL_SIZE = 8,
		ENLARGE_MULTIPLIER = 2,
	};

private:
	void Enlarge();

private:
	IAllocator& m_allocator;
	void* m_pushBuffer = nullptr;
	void* m_pullBuffer = nullptr;
	Event** m_pullEvents = nullptr;
	u8* m_pushPosition = nullptr;
	size_t m_pullSize = 0;
	size_t m_capacity = 0;
};


}

}