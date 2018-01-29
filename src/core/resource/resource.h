#pragma once

#include "core/int.h"
#include "core/file/path.h"

#include "core/file/file.h"///////////////////////////////////////////////////////////Resource depends on file system :-/


namespace Veng
{


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
	const Path& GetPath() { return m_path; }
	State GetState() { return m_state; }

private:
	Path m_path;
	fileHandle m_fileHandle;
	State m_state = State::Empty;
	volatile u32 m_refCount = 0;
};


}