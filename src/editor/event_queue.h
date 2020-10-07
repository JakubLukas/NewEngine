#pragma once

#include "core/int.h"
#include "core/entity.h"
#include "core/resource/resource.h"
#include "core/input/devices/input_device_keyboard.h"
#include "core/input/devices/input_device_mouse.h"


namespace Veng
{

enum class worldId : u32;

class Allocator;


namespace Editor
{

// ----------------------------------------------

enum class EventType : u32
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
	Event(EventType type, u32 size)
		: type(type)
		, size(size)
	{}

	EventType type;
	u32 size = 0;
};


struct EventSelectWorld : public Event
{
	EventSelectWorld(worldId id)
		: Event(EventType::SelectWorld, sizeof(EventSelectWorld))
		, id(id)
	{}

	worldId id;
};

struct EventSelectEntity : public Event
{
	EventSelectEntity(worldId worldId, Entity entity)
		: Event(EventType::SelectEntity, sizeof(EventSelectEntity))
		, worldId(worldId)
		, entity(entity)
	{}

	worldId worldId;
	Entity entity;
};

struct EventSelectCamera : public Event
{
	EventSelectCamera(Entity camera)
		: Event(EventType::SelectCamera, sizeof(EventSelectCamera))
		, camera(camera)
	{}

	Entity camera;
};

struct EventChangedSize : public Event
{
	EventChangedSize(u32 width, u32 height)
		: Event(EventType::ChangedSize, sizeof(EventChangedSize))
		, width(width)
		, height(height)
	{}

	u32 width;
	u32 height;
};

struct EventSelectResource : public Event
{
	EventSelectResource(ResourceType type, resourceHandle resource)
		: Event(EventType::SelectResource, sizeof(EventSelectResource))
		, type(type)
		, resource(resource)
	{}

	ResourceType type;
	resourceHandle resource;
};

// ----------------------------------------------

class EventQueue final
{
public:
	explicit EventQueue(Allocator& allocator);
	~EventQueue();

	void FrameUpdate();

	void PushEvent(const Event& event);

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
	Allocator& m_allocator;
	void* m_pushBuffer = nullptr;
	void* m_pullBuffer = nullptr;
	Event** m_pullEvents = nullptr;
	u8* m_pushPosition = nullptr;
	size_t m_pullSize = 0;
	size_t m_capacity = 0;
};


}

}