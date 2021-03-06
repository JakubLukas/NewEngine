#pragma once

#include "core/int.h"
#include "core/file/path.h"

#include "core/file/file.h"///////////////////////////////////////////////////////////Resource depends on file system :-/


namespace Veng
{


struct ResourceType
{
	explicit ResourceType(const char* name);

	bool operator ==(const ResourceType& other) const { return hash == other.hash; }
	bool operator !=(const ResourceType& other) const { return hash != other.hash; }

	u32 hash;
};


enum class resourceHandle : u64 {};
const resourceHandle INVALID_RESOURCE_HANDLE = (resourceHandle)0;


class Resource
{
	friend class ResourceManager;

public:
	enum class State : u8
	{
		Empty,
		Loading,
		Ready,
		Failure,
	};

public:
	Resource(ResourceType type) : m_type(type) {}

	const Path& GetPath() const { return m_path; }
	State GetState() const { return m_state; }
	void SetState(State state) { m_state = state; }
	ResourceType GetType() const { return m_type; }

private:
	Path m_path;
	fileHandle m_fileHandle = INVALID_FILE_HANDLE;
	ResourceType m_type;
	State m_state = State::Empty;
	volatile u32 m_refCount = 0;
};


}