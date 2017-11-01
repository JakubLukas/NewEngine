#include "irenderer.h"

#include "core/allocators.h"
#include "core/engine.h"
#include "core/associative_array.h"
#include "core/logs.h"

#include <bgfx/bgfx.h>///////////////
//#include <bgfx/platform.h>///////////////

#include <math.h>
#include "core/memory.h"
#include "core/file/file_system.h"
namespace bx
{
/// Abstract allocator interface.
///
struct AllocatorI
{
	///
	virtual ~AllocatorI() { }

	/// Allocates, resizes memory block, or frees memory.
	///
	/// @param[in] _ptr If _ptr is NULL new block will be allocated.
	/// @param[in] _size If _ptr is set, and _size is 0, memory will be freed.
	/// @param[in] _align Alignment.
	/// @param[in] _file Debug file path info.
	/// @param[in] _line Debug file line info.
	virtual void* realloc(void* _ptr, size_t _size, size_t _align, const char* _file, uint32_t _line) = 0;
};



void mtxRotateXY(float* _result, float _ax, float _ay);
struct Handness
{
	enum Enum
	{
		Left,
		Right,
	};
};
float ftan(float _a);
const float kPi = 3.1415926535897932384626433832795f;
inline float toRad(float _deg)
{
	return _deg * kPi / 180.0f;
}
template<Handness::Enum HandnessT>
void mtxProjXYWH(float* _result, float _x, float _y, float _width, float _height, float _near, float _far, bool _oglNdc);
template<Handness::Enum HandnessT>
void mtxProjImpl(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc);
void mtxProj(float* _result, float _fovy, float _aspect, float _near, float _far, bool _oglNdc);
inline void vec3Cross(float* _result, const float* _a, const float* _b)
{
	_result[0] = _a[1] * _b[2] - _a[2] * _b[1];
	_result[1] = _a[2] * _b[0] - _a[0] * _b[2];
	_result[2] = _a[0] * _b[1] - _a[1] * _b[0];
}
inline float vec3Dot(const float* _a, const float* _b)
{
	return _a[0] * _b[0] + _a[1] * _b[1] + _a[2] * _b[2];
}
float fsqrt(float _a);
inline float vec3Length(const float* _a)
{
	return fsqrt(vec3Dot(_a, _a));
}
inline float vec3Norm(float* _result, const float* _a)
{
	const float len = vec3Length(_a);
	const float invLen = 1.0f / len;
	_result[0] = _a[0] * invLen;
	_result[1] = _a[1] * invLen;
	_result[2] = _a[2] * invLen;
	return len;
}
inline void vec3Sub(float* _result, const float* _a, const float* _b)
{
	_result[0] = _a[0] - _b[0];
	_result[1] = _a[1] - _b[1];
	_result[2] = _a[2] - _b[2];
}
void mtxLookAtImpl(float* _result, const float* _eye, const float* _view, const float* _up);
void mtxLookAtLh(float* _result, const float* _eye, const float* _at, const float* _up);
void mtxLookAt(float* _result, const float* _eye, const float* _at, const float* _up = NULL);

inline void mtxQuat(float* _result, const float* _quat)
{
	const float x = _quat[0];
	const float y = _quat[1];
	const float z = _quat[2];
	const float w = _quat[3];

	const float x2 = x + x;
	const float y2 = y + y;
	const float z2 = z + z;
	const float x2x = x2 * x;
	const float x2y = x2 * y;
	const float x2z = x2 * z;
	const float x2w = x2 * w;
	const float y2y = y2 * y;
	const float y2z = y2 * z;
	const float y2w = y2 * w;
	const float z2z = z2 * z;
	const float z2w = z2 * w;

	_result[0] = 1.0f - (y2y + z2z);
	_result[1] = x2y - z2w;
	_result[2] = x2z + y2w;
	_result[3] = 0.0f;

	_result[4] = x2y + z2w;
	_result[5] = 1.0f - (x2x + z2z);
	_result[6] = y2z - x2w;
	_result[7] = 0.0f;

	_result[8] = x2z - y2w;
	_result[9] = y2z + x2w;
	_result[10] = 1.0f - (x2x + y2y);
	_result[11] = 0.0f;

	_result[12] = 0.0f;
	_result[13] = 0.0f;
	_result[14] = 0.0f;
	_result[15] = 1.0f;
}

inline void mtxQuatTranslation(float* _result, const float* _quat, const float* _translation)
{
	mtxQuat(_result, _quat);
	_result[12] = -(_result[0] * _translation[0] + _result[4] * _translation[1] + _result[8] * _translation[2]);
	_result[13] = -(_result[1] * _translation[0] + _result[5] * _translation[1] + _result[9] * _translation[2]);
	_result[14] = -(_result[2] * _translation[0] + _result[6] * _translation[1] + _result[10] * _translation[2]);
}

inline void mtxQuatTranslationHMD(float* _result, const float* _quat, const float* _translation)
{
	float quat[4];
	quat[0] = -_quat[0];
	quat[1] = -_quat[1];
	quat[2] = _quat[2];
	quat[3] = _quat[3];
	mtxQuatTranslation(_result, quat, _translation);
}













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


///////////////
struct BGFXAllocator : public bx::AllocatorI
{
	static const size_t NATURAL_ALIGNEMENT = 8;

	explicit BGFXAllocator(IAllocator& source)
		: m_source(source)
	{
	}

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
	struct BgfxCallback : public bgfx::CallbackI
	{
		BgfxCallback() {}
		~BgfxCallback() {}
		void fatal(bgfx::Fatal::Enum _code, const char* _str) override
		{
			switch (_code)
			{
			case bgfx::Fatal::DebugCheck:
				LogError("Error: bgfx: DebugCheck: %s\n", _str);
				break;
			case bgfx::Fatal::InvalidShader:
				LogError("Error: bgfx: InvalidShader: %s\n", _str);
				break;
			case bgfx::Fatal::UnableToInitialize:
				LogError("Error: bgfx: UnableToInitialize: %s\n", _str);
				break;
			case bgfx::Fatal::UnableToCreateTexture:
				LogError("Error: bgfx: UnableToCreateTexture: %s\n", _str);
				break;
			case bgfx::Fatal::DeviceLost:
				LogError("Error: bgfx: DeviceLost: %s\n", _str);
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

public:
	RenderSystemImpl(Engine& engine)
		: m_engine(engine)
		//, m_allocator(HeapAllocator(engine.GetAllocator()))//TODO, how to make custom allocator per system
		, m_allocator(engine.GetAllocator())
		, m_bgfxAllocator(m_allocator)
		, m_meshes(m_allocator)
	{
		///////////////
		bgfx::PlatformData d = { 0 };
		d.nwh = m_engine.GetPlatformData().windowHndl;
		bgfx::setPlatformData(d);

		bgfx::init(bgfx::RendererType::Enum::Direct3D12, BGFX_PCI_ID_NONE, 0, &m_bgfxCallback, &m_bgfxAllocator);
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
		);

		// Create program from shaders.
		FS::FileMode mode{
			FS::FileMode::Access::Read,
			FS::FileMode::ShareMode::NoShare,
			FS::FileMode::CreationDisposition::OpenExisting,
			FS::FileMode::FlagNone
		};
		FS::File fileVS;
		ASSERT(fileVS.Open("D:/projects/NewEngine/shaders/dx11/vs_cubes.bin", mode));
		size_t fileVSSize = fileVS.GetSize();
		u8* fileVSData = (u8*)m_allocator.Allocate(fileVSSize, 1);
		const bgfx::Memory* memVS = bgfx::alloc((u32)fileVSSize + 1);
		ASSERT(fileVS.Read(memVS->data, fileVSSize));
		memVS->data[memVS->size - 1] = '\0';
		fileVS.Close();
		FS::File fileFS;
		ASSERT(fileFS.Open("D:/projects/NewEngine/shaders/dx11/fs_cubes.bin", mode));
		size_t fileFSSize = fileFS.GetSize();
		u8* fileFSData = (u8*)m_allocator.Allocate(fileFSSize, 1);
		const bgfx::Memory* memFS = bgfx::alloc((u32)fileFSSize + 1);
		ASSERT(fileFS.Read(memFS->data, fileFSSize));
		memFS->data[memFS->size - 1] = '\0';
		fileFS.Close();
		bgfx::ShaderHandle fsh = bgfx::createShader(memFS);
		bgfx::ShaderHandle vsh = bgfx::createShader(memVS);
		bgfx::setName(fsh, "shaders/dx11/fs_cubes.bin");
		bgfx::setName(vsh, "shaders/dx11/vs_cubes.bin");
		m_program = bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);

		///////////////
	}


	~RenderSystemImpl() override
	{
		bgfx::destroy(m_ibh);
		bgfx::destroy(m_vbh);
		bgfx::destroy(m_program);

		// Shutdown bgfx.
		bgfx::shutdown();
	}


	void Update(float deltaTime) override
	{
		static float time = 0;
		time += deltaTime * 0.001f;
		static const u32 m_width = 600;
		static const u32 m_height = 400;
		float at[3] = { 0.0f, 0.0f,   0.0f };
		float eye[3] = { 0.0f, 0.0f, -35.0f };

		const bgfx::HMD* hmd = bgfx::getHMD();
		if (NULL != hmd && 0 != (hmd->flags & BGFX_HMD_RENDERING))
		{
			float view[16];
			bx::mtxQuatTranslationHMD(view, hmd->eye[0].rotation, eye);
			bgfx::setViewTransform(0, view, hmd->eye[0].projection, BGFX_VIEW_STEREO, hmd->eye[1].projection);

			// Set view 0 default viewport.
			//
			// Use HMD's width/height since HMD's internal frame buffer size
			// might be much larger than window size.
			bgfx::setViewRect(0, 0, 0, hmd->width, hmd->height);
		}
		else
		{
			float view[16];
			bx::mtxLookAt(view, eye, at);

			float proj[16];
			bx::mtxProj(proj, 60.0f, float(m_width) / float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
			bgfx::setViewTransform(0, view, proj);

			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));
		}

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::touch(0);

		// Submit 11x11 cubes.
		for (uint32_t yy = 0; yy < 11; ++yy)
		{
			for (uint32_t xx = 0; xx < 11; ++xx)
			{
				float mtx[16];
				bx::mtxRotateXY(mtx, time + xx*0.21f, time + yy*0.37f);
				mtx[12] = -15.0f + float(xx)*3.0f;
				mtx[13] = -15.0f + float(yy)*3.0f;
				mtx[14] = 0.0f;

				// Set model matrix for rendering.
				bgfx::setTransform(mtx);

				// Set vertex and index buffer.
				bgfx::setVertexBuffer(0, m_vbh);
				bgfx::setIndexBuffer(m_ibh);

				// Set render states.
				bgfx::setState(0
					| BGFX_STATE_DEFAULT
					| BGFX_STATE_PT_TRISTRIP
				);

				// Submit primitive for rendering to view 0.
				bgfx::submit(0, m_program);
			}
		}

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		bgfx::frame();
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
	BGFXAllocator m_bgfxAllocator;
	BgfxCallback m_bgfxCallback;
	bgfx::VertexBufferHandle m_vbh;
	bgfx::IndexBufferHandle m_ibh;
	bgfx::ProgramHandle m_program;
	int64_t m_timeOffset;
	/////////////////////
};





RenderSystem* RenderSystem::Create(Engine& engine)
{
	return new (engine.GetAllocator(), ALIGN_OF(RenderSystemImpl)) RenderSystemImpl(engine);
}

void RenderSystem::Destroy(RenderSystem* system)
{
	RenderSystemImpl* p = (RenderSystemImpl*)system;
	IAllocator& allocator = system->GetEngine().GetAllocator();
	p->~RenderSystemImpl();
	allocator.Deallocate(p);
}

}