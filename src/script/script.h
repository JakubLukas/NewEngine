#pragma once

#include "core/isystem.h"


namespace Veng
{

class Engine;


class ScriptSystem : public ISystem
{
public:
	static ScriptSystem* Create(Engine& engine);
	static void Destroy(ScriptSystem* system);

public:
	virtual ~ScriptSystem() override {}

	virtual void Init() override = 0;

	virtual void Update(float deltaTime) override = 0;
	virtual const char* GetName() const override = 0;

	virtual IScene* GetScene(worldId world) const override = 0;

	virtual void WorldAdded(worldId world) override = 0;
	virtual void WorldRemoved(worldId world) override = 0;

	virtual Engine& GetEngine() const = 0;
};


}