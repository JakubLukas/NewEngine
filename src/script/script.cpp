#include "script.h"

#include "core/engine.h"
#include "core/allocators.h"

#include "core/world/world.h"//////////////
#include "renderer/renderer.h"////////////
#include "core/math/math.h"///////////////////
#include "core/file/path.h"/////////////
#include "core/math/quaternion.h"///////////
#include "core/math/matrix.h"//////////////
#include "core/time.h"///////////////


namespace Veng
{


class ScriptSystemImpl : public ScriptSystem
{
public:
	ScriptSystemImpl(Engine& engine)
		: m_engine(engine)
	{
	}
	~ScriptSystemImpl() override
	{

	}

	void Init() override
	{
		m_world = m_engine.AddWorld();
		World* world = m_engine.GetWorld(m_world);
		RenderScene* renderScene = static_cast<RenderScene*>(m_engine.GetSystem("renderer")->GetScene());

		int i = 0;
		for (unsigned yy = 0; yy < 11; ++yy)
		{
			for (unsigned xx = 0; xx < 11; ++xx)
			{
				m_entities[i] = world->CreateEntity();
				Transform& trans = world->GetEntityTransform(m_entities[i]);
				if(i % 2 == 0)
					renderScene->AddModelComponent(m_entities[i], m_world, "models/cubes.model");
				else
					renderScene->AddModelComponent(m_entities[i], m_world, "models/pyramid.model");

				Quaternion rot = Quaternion::IDENTITY;
				rot = rot * Quaternion(Vector3::AXIS_X, xx*0.21f);
				rot = rot * Quaternion(Vector3::AXIS_Y, yy*0.37f);
				Vector3 pos = {
					-15.0f + float(xx) * 3.0f,
					-15.0f + float(yy) * 3.0f,
					0.0f
				};
				trans = Transform(rot, pos);
				i++;
			}
		}
	}

	void Update(float deltaTime) override
	{
		Quaternion rot = Quaternion::IDENTITY;
		rot = rot * Quaternion(Vector3::AXIS_X, SecFromMSec(deltaTime));

		World* world = m_engine.GetWorld(m_world);

		for (int i = 0; i < 121; ++i)
		{
			Transform& trans = world->GetEntityTransform(m_entities[i]);
			trans.rotation = trans.rotation * rot;
		}
	}


	const char* GetName() const override { return "script"; }

	IScene* GetScene() const override { return nullptr; }

	Engine& GetEngine() const override { return m_engine; }

private:
	Engine& m_engine;

	worldId m_world;
	Entity m_entities[121];
};


ScriptSystem* ScriptSystem::Create(Engine& engine)
{
	return NEW_OBJECT(engine.GetAllocator(), ScriptSystemImpl)(engine);
}

void ScriptSystem::Destroy(ScriptSystem* system)
{
	IAllocator& allocator = system->GetEngine().GetAllocator();
	DELETE_OBJECT(allocator, system);
}


}