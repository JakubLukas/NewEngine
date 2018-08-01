#include "editor.h"

#include <bgfx/bgfx.h>

#include "core/allocators.h"
#include "core/asserts.h"
#include "core/engine.h"
#include "core/logs.h"


#include "renderer/irenderer.h"////////////////////////
#include "core/file/path.h"////////////////////////////////
#include "core/math.h"////////////////////////////////

#include <bgfx/bgfx.h>///////////////


namespace bx
{

/// Abstract allocator interface.
struct AllocatorI
{
	virtual ~AllocatorI() {}

	/// Allocates, resizes memory block, or frees memory.
	///
	/// @param[in] _ptr If _ptr is NULL new block will be allocated.
	/// @param[in] _size If _ptr is set, and _size is 0, memory will be freed.
	/// @param[in] _align Alignment.
	/// @param[in] _file Debug file path info.
	/// @param[in] _line Debug file line info.
	virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line) = 0;
};

}


namespace bgfx
{

struct PlatformData
{
	void* ndt;          //!< Native display type.
	void* nwh;          //!< Native window handle.
	void* context;      //!< GL context, or D3D device.
	void* backBuffer;   //!< GL backbuffer, or D3D render target view.
	void* backBufferDS; //!< Backbuffer depth/stencil.
	void* session;      //!< ovrSession, for Oculus SDK
};

void setPlatformData(const PlatformData& _data);

bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName);

}


namespace Veng
{

struct BGFXAllocator : public bx::AllocatorI
{
	static const size_t NATURAL_ALIGNEMENT = 8;

	explicit BGFXAllocator(IAllocator& source)
		: m_source(source)
	{}

	void* realloc(void* _ptr, size_t _size, size_t _alignment, const char* _file, u32 _line) override
	{
		if (_size == 0)
		{
			if (_ptr != nullptr)
			{
				m_source.Deallocate(_ptr);
			}
			return nullptr;
		}
		else
		{
			_alignment = (_alignment < NATURAL_ALIGNEMENT) ? NATURAL_ALIGNEMENT : _alignment;

			if (_ptr == nullptr)
			{
				return m_source.Allocate(_size, _alignment);
			}
			else
			{
				return m_source.Reallocate(_ptr, _size, _alignment);
			}
		}
	}


	IAllocator& m_source;
};


struct BGFXCallback : public bgfx::CallbackI
{
	void fatal(bgfx::Fatal::Enum _code, const char* _str) override
	{
		switch (_code)
		{
		case bgfx::Fatal::DebugCheck:
			Log(LogType::Error, "Error: bgfx: DebugCheck: %s\n", _str);
			break;
		case bgfx::Fatal::InvalidShader:
			Log(LogType::Error, "Error: bgfx: InvalidShader: %s\n", _str);
			break;
		case bgfx::Fatal::UnableToInitialize:
			Log(LogType::Error, "Error: bgfx: UnableToInitialize: %s\n", _str);
			break;
		case bgfx::Fatal::UnableToCreateTexture:
			Log(LogType::Error, "Error: bgfx: UnableToCreateTexture: %s\n", _str);
			break;
		case bgfx::Fatal::DeviceLost:
			Log(LogType::Error, "Error: bgfx: DeviceLost: %s\n", _str);
			break;
		}
	}
	void traceVargs(const char* _filePath, uint16_t _line, const char* _format, va_list _argList) override {}
	void profilerBegin(const char* _name, uint32_t _abgr, const char* _filePath, uint16_t _line) override {}
	void profilerBeginLiteral(const char* _name, uint32_t _abgr, const char* _filePath, uint16_t _line) override {}
	void profilerEnd() override {}
	uint32_t cacheReadSize(uint64_t _id) override { return 0; }
	bool cacheRead(uint64_t _id, void* _data, uint32_t _size) override { return false; }
	void cacheWrite(uint64_t _id, const void* _data, uint32_t _size) override {}
	void screenShot(const char* _filePath, uint32_t _width, uint32_t _height, uint32_t _pitch, const void* _data, uint32_t _size, bool _yflip) override {}
	void captureBegin(uint32_t _width, uint32_t _height, uint32_t _pitch, bgfx::TextureFormat::Enum _format, bool _yflip) override {}
	void captureEnd() override {}
	void captureFrame(const void* _data, uint32_t _size) override {}
};

}


namespace Veng
{

RenderSystem* m_tmpPlugRender = nullptr;////////////////////////////////

class EditorImpl : public Editor
{
public:
	EditorImpl(IAllocator& allocator, App& app)
		: m_allocator(allocator)
		, m_app(app)
		, m_bgfxAllocator(m_allocator)
	{

	}

	void Init() override
	{
		InitRender();

		m_engine = Engine::Create(m_allocator);
		Engine::PlatformData platformData;
		platformData.windowHndl = m_subHwnd;
		m_engine->SetPlatformData(platformData);
		InitPlugins();


		//DUMMY GAMEPLAY CODE //////////////////////////////////////////////////////////////////////////////////////////
		worldId worldHandle = m_engine->AddWorld();
		World* world = m_engine->GetWorld(worldHandle);
		RenderScene* renderScene = static_cast<RenderScene*>(m_tmpPlugRender->GetScene());
		Entity camEnt = world->CreateEntity();
		Transform& camTrans = world->GetEntityTransform(camEnt);
		renderScene->AddCameraComponent(camEnt, worldHandle, 60.0_deg, 0.001f, 100.0f);

		for (unsigned yy = 0; yy < 11; ++yy)
		{
			for (unsigned xx = 0; xx < 11; ++xx)
			{
				Entity entity = world->CreateEntity();
				Transform& trans = world->GetEntityTransform(entity);
				renderScene->AddModelComponent(entity, worldHandle, "models/cubes.model");

				Quaternion rot = Quaternion::IDENTITY;
				//rot = rot * Quaternion(Vector3::AXIS_X, xx*0.21f);
				//rot = rot * Quaternion(Vector3::AXIS_Y, yy*0.37f);
				Vector3 pos = {
					-15.0f + float(xx) * 3.0f,
					-15.0f + float(yy) * 3.0f,
					0.0f
				};
				trans = Transform(rot, pos);
			}
		}
	}

	void Deinit() override
	{
		//TODO: shut down engine gracefully
		RenderSystem::Destroy(m_tmpPlugRender);
		Engine::Destroy(m_engine, m_allocator);

		DeinitRender();
	}

	void Update() override
	{
		UpdateRender();

		bgfx::setViewFrameBuffer(1, m_fbh);
		bgfx::touch(1);
		m_engine->Update(1000 / 60);

		bgfx::frame();//flip buffers
	}

	void Resize(u32 width, u32 height) override
	{
		if(m_tmpPlugRender)
			m_tmpPlugRender->Resize(width, height);
	}

	Engine* GetEngine() override { return m_engine; }


	void InitPlugins()
	{
		ASSERT(m_engine != nullptr);
		m_tmpPlugRender = RenderSystem::Create(*m_engine);
		m_tmpPlugRender->Init();
		WindowSize size = m_app.GetWindowSize(m_subHwnd);
		m_tmpPlugRender->Resize(size.width, size.height);
		m_engine->AddPlugin(m_tmpPlugRender);
	}

	void InitRender()
	{
		void* hwnd = m_app.GetMainWindowHandle();
		m_subHwnd = m_app.CreateSubWindow();
		ASSERT(m_subHwnd != nullptr);

		bgfx::PlatformData d{ 0 };
		d.nwh = hwnd;
		bgfx::setPlatformData(d);

		bgfx::Init bgfxInit;
		bgfxInit.type = bgfx::RendererType::Count;
		bgfxInit.vendorId = BGFX_PCI_ID_NONE;
		bgfxInit.deviceId = 0;
		bgfxInit.debug = false;
		bgfxInit.profile = false;
		//bgfxInit.resolution;
		//bgfxInit.limits;
		bgfxInit.callback = &m_bgfxCallback;
		bgfxInit.allocator = &m_bgfxAllocator;

		bgfx::init(bgfxInit);

		bgfx::setDebug(BGFX_DEBUG_NONE);//TODO

		m_fbh = bgfx::createFrameBuffer(m_subHwnd, uint16_t(100), uint16_t(100));

		bgfx::setViewRect(0, 0, 0, uint16_t(800), uint16_t(600));
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x30ff30ff
			, 1.0f
			, 0
		);

		bgfx::setViewRect(1, 0, 0, uint16_t(100), uint16_t(100));
		bgfx::setViewClear(1
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
		);
	}

	void UpdateRender()
	{
		bgfx::setViewFrameBuffer(0, BGFX_INVALID_HANDLE);
		bgfx::touch(0);
	}

	void DeinitRender()
	{
		bgfx::shutdown();
		m_app.DestroySubWindow(m_subHwnd);
	}


private:
	IAllocator& m_allocator;
	App& m_app;
	Engine* m_engine = nullptr;

	BGFXAllocator m_bgfxAllocator;
	BGFXCallback m_bgfxCallback;

	bgfx::FrameBufferHandle m_fbh;

	void* m_subHwnd = nullptr;///////////////////////////////////
};


Editor* Editor::Create(IAllocator& allocator, App& app)
{
	return NEW_OBJECT(allocator, EditorImpl)(allocator, app);
}

void Editor::Destroy(Editor* editor, IAllocator& allocator)
{
	DELETE_OBJECT(allocator, editor);
}

}
