#pragma once

#include "core/int.h"
#include "core/allocators.h"
#include "core/vector3.h"
#include "core/associative_array.h"


namespace Veng
{


/*
InputController Stingray functions

constructor(alloc, inputManager)
destructor
update(float)
flushState()
type
category
name
active
connected (right now)
disconnected(right now)
#buttons
getButton
anyButton
buttonName
buttonLocaleName
buttonId(string)
hasButton(string)
pressedMask ??
releasedMask ??
#axes
getAxis
deadZone
setDeadZone
axisName
axisId(string)
hasAxis(string)
*/
class InputController
{
public:
	enum class Type : u8
	{
		MOUSE,
		KEYBOARD
	};

	InputController();
	virtual ~InputController();

	void Clear();
	Type GetType() const { return m_type; }
	//const char* GetName()

	bool IsActive() const;
	bool Connected() const;
	bool Disconnected() const;

	virtual u8 GetButtonsCount() const;

	virtual u8 GetAxisCount() const;

protected:
	Type m_type;
};


class InputSystem//make pure virtual interface
{
public:
	explicit InputSystem(IAllocator& allocator);

private:
	IAllocator& m_allocator;
	AssociativeArray<void*, InputController> m_controllers;
};


}