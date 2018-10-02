#pragma once

#include "core/int.h"
#include "core/file/path.h"

#include "core/file/file.h"///////////////////////////////////////////////////////////Resource depends on file system :-/


namespace Veng
{


enum class ResourceType
{
	ShaderInternal,
	Shader,
	Material,
	Model,
	Texture,
};


enum class resourceHandle : u64 {};
const resourceHandle INVALID_HANDLE = static_cast<resourceHandle>(0);


class Resource
{
	friend class ResourceManager;

public:
	enum class State
	{
		Empty,
		Loading,
		Ready,
		Failure,
	};

public:
	const Path& GetPath() const { return m_path; }
	State GetState() const { return m_state; }
	void SetState(State state) { m_state = state; }

private:
	Path m_path;
	fileHandle m_fileHandle;
	State m_state = State::Empty;
	volatile u32 m_refCount = 0;
};


}