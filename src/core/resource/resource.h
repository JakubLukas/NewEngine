#pragma once

#include "core/int.h"
#include "core/file/path.h"


namespace Veng
{


class Resource
{
	friend class NewShaderManager;
	friend class ResourceManager;

public:
	enum class State
	{
		Empty,
		Loading,
		Ready,
		Failure,
	};

private:
	Path m_path;
	State m_state = State::Empty;
	volatile u32 m_refCount = 0;
};


}