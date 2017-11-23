#pragma once

#include "core/int.h"
#include "core/file/path.h"


namespace Veng
{


class Resource
{
public:
	enum State
	{
		Empty,
		Loading,
		Ready,
		Failure,
	};

	void Load(Path path);

private:
	Path m_path;
	volatile u32 m_refCount = 0;
};


}