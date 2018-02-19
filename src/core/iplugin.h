#pragma once


namespace Veng
{


enum class componentHandle : u64 {};


class IScene
{
public:
	virtual ~IScene() {}

	virtual void Update(float deltaTime) = 0;
};


class IPlugin
{
public:
	virtual ~IPlugin() {}

	virtual void Init() = 0;

	virtual void Update(float deltaTime) = 0;
	virtual const char* GetName() const = 0;

	virtual IScene* GetScene() const = 0;
};


}