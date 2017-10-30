#include "irenderer.h"

#include "core/allocators.h"
#include "core/engine.h"
#include "core/associative_array.h"

#include "bgfx/bgfx.h"///////////////


namespace Veng
{


///////////////
struct PosColorVertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_abgr;

	static void init()
	{
		ms_decl
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
	};

	static bgfx::VertexDecl ms_decl;
};

bgfx::VertexDecl PosColorVertex::ms_decl;

static PosColorVertex s_cubeVertices[] =
{
	{ -1.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{ -1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
	{ -1.0f,  1.0f, -1.0f, 0xffff0000 },
	{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
	{ -1.0f, -1.0f, -1.0f, 0xffffff00 },
	{ 1.0f, -1.0f, -1.0f, 0xffffffff },
};

static const uint16_t s_cubeTriList[] =
{
	0, 1, 2, // 0
	1, 3, 2,
	4, 6, 5, // 2
	5, 6, 7,
	0, 2, 4, // 4
	4, 2, 6,
	1, 5, 3, // 6
	5, 7, 3,
	0, 4, 1, // 8
	4, 5, 1,
	2, 3, 6, // 10
	6, 3, 7,
};

static const uint16_t s_cubeTriStrip[] =
{
	0, 1, 2,
	3,
	7,
	1,
	5,
	0,
	4,
	2,
	6,
	7,
	4,
	5,
};
///////////////



class RenderSystemImpl : public RenderSystem
{
public:
	RenderSystemImpl(Engine& engine)
		: m_engine(engine)
		, m_allocator(HeapAllocator(engine.GetAllocator()))
		, m_meshes(m_allocator)
	{
		///////////////
		/*bgfx::init(bgfx::RendererType::Enum::Direct3D12, BGFX_PCI_ID_NVIDIA);
		bgfx::reset(600, 400, BGFX_RESET_VSYNC);

		bgfx::setDebug(BGFX_DEBUG_NONE);

		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
		);

		// Create vertex stream declaration.
		PosColorVertex::init();

		// Create static vertex buffer.
		m_vbh = bgfx::createVertexBuffer(
			// Static data can be passed with bgfx::makeRef
			bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices))
			, PosColorVertex::ms_decl
		);

		// Create static index buffer.
		m_ibh = bgfx::createIndexBuffer(
			// Static data can be passed with bgfx::makeRef
			bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip))
		);*/

		// Create program from shaders.
		//m_program = loadProgram("vs_cubes", "fs_cubes");

		//m_timeOffset = bx::getHPCounter();

		///////////////
	}


	~RenderSystemImpl() override
	{
		/*bgfx::destroy(m_ibh);
		bgfx::destroy(m_vbh);
		//bgfx::destroy(m_program);

		// Shutdown bgfx.
		bgfx::shutdown();*/
	}


	void Update(float deltaTime) override
	{

	};


	const char* GetName() const override { return "renderer"; }


	void AddMeshComponent(Entity entity, worldId world) override
	{
		m_meshes.Insert(entity, Mesh());
	}

	void RemoveMeshComponent(Entity entity, worldId world) override
	{
		m_meshes.Erase(entity);
	}

	bool HasMeshComponent(Entity entity, worldId world) override
	{
		Mesh* mesh;
		return m_meshes.Find(entity, mesh);
	}


	Engine& GetEngine() const override { return m_engine; }

private:
	Engine& m_engine;
	IAllocator& m_allocator;
	AssociativeArray<Entity, Mesh> m_meshes;

	/////////////////////
	//bgfx::VertexBufferHandle m_vbh;
	//bgfx::IndexBufferHandle m_ibh;
	//bgfx::ProgramHandle m_program;
	//int64_t m_timeOffset;
	/////////////////////
};


RenderSystem* RenderSystem::Create(Engine& engine)
{
	return (RenderSystem*)new (NewPlaceholder(), engine.GetAllocator()) RenderSystemImpl(engine);
}

void RenderSystem::Destroy(RenderSystem* system)
{
	RenderSystemImpl* p = (RenderSystemImpl*)system;
	IAllocator& allocator = system->GetEngine().GetAllocator();
	p->~RenderSystemImpl();
	allocator.Deallocate(p);
}

}