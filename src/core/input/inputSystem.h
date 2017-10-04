#pragma once

#include "core/int.h"
#include "core/allocators.h"
#include "core/vector3.h"


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
	//virtual 
	/*
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
protected:
	Type m_type;
};


class InputSystem
{
	
};